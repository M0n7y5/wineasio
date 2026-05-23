# PipeASIO

PipeASIO is an ASIO driver for Wine that talks to PipeWire directly — no `libjack.so.0` runtime dependency.

It's a fork of [WineASIO](https://github.com/wineasio/wineasio), created so the driver loads cleanly inside the Steam Runtime `steamrt4` container that FL Studio runs in under Faugus / Proton-CachyOS — that container ships `libpipewire-0.3` but not `libjack.so.0`, which makes upstream WineASIO SEGV on `dlopen`.

PipeASIO has its own distinct COM identity — CLSID `{2D3CA9E2-1193-4C5D-B5FD-38798F3DC074}`, registry paths `HKCU\Software\Wine\PipeASIO` and `HKCU\Software\ASIO\PipeASIO`, DLL filename `pipeasio64.dll`. Installing PipeASIO alongside WineASIO is safe; neither overrides the other, and hosts (FL Studio etc.) see them as separate ASIO drivers.

ASIO is the most common Windows low-latency driver, so is commonly used in audio workstation programs like FL Studio, Ableton Live, and Reaper.

![Screenshot](screenshot.png)

### BUILDING

This fork uses CMake. x86_64 only.

Build requirements: `cmake` (≥ 3.20), `ninja-build` (recommended) or GNU make,
`gcc`, Wine SDK (`wine-devel` / `winehq-stable-dev`), `pkg-config`, and
`libpipewire-0.3-dev` (declared at configure time; native PipeWire backend
is in progress).

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Debug build with assertions and Wine debug channel macros:

```sh
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

### INSTALLING

Default install prefix is `/usr/local`; pass `--prefix /usr` to match the
distro layout most ASIO hosts expect.

```sh
sudo cmake --install build --prefix /usr
```

This lays down:

```
/usr/lib/wine/x86_64-windows/pipeasio64.dll
/usr/lib/wine/x86_64-windows/pipeasio.dll        -> pipeasio64.dll
/usr/lib/wine/x86_64-unix/pipeasio64.dll.so
/usr/lib/wine/x86_64-unix/pipeasio.dll.so        -> pipeasio64.dll.so
```

The `pipeasio.dll{,.so}` symlinks satisfy the unified PE name that Wine 10+
expects; without them `regsvr32 pipeasio64.dll` fails with status `c0000135`
on newer Wine.

**NOTE:** Wine library directories vary across distros — adjust `--prefix`
or override `CMAKE_INSTALL_LIBDIR` if your distro is non-standard.

### DEVELOPMENT (VS Code)

Recommended extensions are listed in `.vscode/extensions.json`; VS Code
prompts on first open. The build emits `build/compile_commands.json` for
clangd; the in-tree `.clang-format` and `.editorconfig` keep diffs clean.

#### EXTRAS

For user convenience a `pipeasio-register` script is included in this repo, if you are packaging PipeASIO consider installing it as part of PipeASIO.

Additionally a control panel GUI is provided in this repository's `gui` subdir, which requires PyQt6 or PyQt5 to build and run.  
The PipeASIO driver will use this GUI as the ASIO control panel.

### REGISTERING

After building and installing PipeASIO, we still need to register it on each Wine prefix.  
For your convenience a script is provided on this repository, so you can simply run:

```sh
pipeasio-register
```

to activate PipeASIO for the current Wine prefix.

#### CUSTOM WINEPREFIX

The `pipeasio-register` script will register the PipeASIO driver in the default Wine prefix `~/.wine`.  
You can specify another prefix like so:

```sh
env WINEPREFIX=~/asioapp pipeasio-register
```

### GENERAL INFORMATION

PipeASIO talks to PipeWire 1.6+ natively via `libpipewire-0.3`. The graph
quantum and sample rate are locked to the ASIO host's negotiated values
via `PW_KEY_NODE_FORCE_QUANTUM` / `PW_KEY_NODE_FORCE_RATE`.

The configuration of PipeASIO is done with Windows registry (`HKEY_CURRENT_USER\Software\Wine\PipeASIO`).  
All these options can be overridden by environment variables.  
There is also a GUI for changing these settings, which PipeASIO will try to launch when the ASIO "panel" is clicked.

The registry keys are automatically created with default values if they doesn't exist when the driver initializes.
The available options are:

#### [Number of inputs] & [Number of outputs]
These two settings control the number of PipeWire DSP ports that PipeASIO will try to open.  
Defaults are 16 in and 16 out.  Environment variables are `PIPEASIO_NUMBER_INPUTS` and `PIPEASIO_NUMBER_OUTPUTS`.

#### [Autostart server]

Honored as a no-op under PipeWire — the daemon is always running, so
there is no equivalent of JACK's "start the server" behavior. The
registry key is retained so existing migration scripts can write to it
without erroring; PipeASIO ignores its value.

#### [Connect to hardware]
Defaults to on (1), makes PipeASIO try to connect the ASIO channels to the physical I/O ports on your hardware.  
Setting it to 0 disables it.  
The environment variable is `PIPEASIO_CONNECT_TO_HARDWARE`, and it can be set to on or off.

#### [Fixed buffersize]
Defaults to on (1) which means the buffer size is controlled by PipeWire and PipeASIO has no say in the matter.  
When set to 0, an ASIO app will be able to change PipeWire's quantum (via `PW_KEY_NODE_FORCE_QUANTUM`) when calling `CreateBuffers()`.  
The environment variable is `PIPEASIO_FIXED_BUFFERSIZE` and it can be set to on or off.

#### [Preferred buffersize]
Defaults to 1024, and is one of the sizes returned by `GetBufferSize()`, see the ASIO documentation for details.  
Must be a power of 2.

The other values returned by the driver are hardcoded in the source,  
see `ASIO_MINIMUM_BUFFERSIZE` which is set at 16, and `ASIO_MAXIMUM_BUFFERSIZE` which is set to 8192.  
The environment variable is `PIPEASIO_PREFERRED_BUFFERSIZE`.

Be careful, if you set a size that isn't supported by the hardware, PipeWire will either reject the request or insert resampling — either way you may get xruns. Consider changing `ASIO_MINIMUM_BUFFERSIZE` / `ASIO_MAXIMUM_BUFFERSIZE` to values you know work on your system before building.

In addition there is a `PIPEASIO_CLIENT_NAME` environment variable,
that overrides the PipeWire client/node name derived from the program name.

### CHANGE LOG

#### 1.3.0
* 24-JUL-2025: Make GUI settings panel compatible with PyQt6 or PyQt5
* 17-JUL-2025: Load libjack.so.0 dynamically at runtime, removing build dep
* 17-JUL-2025: Remove useless -mnocygwin flag
* 28-JUN-2025: Remove dependency on asio headers

#### 1.2.0
* 29-SEP-2023: Fix compatibility with Wine > 8
* 29-SEP-2023: Add pipeasio-register script for simplifying driver registration

#### 1.1.0
* 18-FEB-2022: Various bug fixes (falkTX)
* 24-NOV-2021: Fix compatibility with Wine > 6.5

#### 1.0.0
* 14-JUL-2020: Add packaging script
* 12-MAR-2020: Fix control panel startup
* 08-FEB-2020: Fix code to work with latest Wine
* 08-FEB-2020: Add custom GUI for PipeASIO settings, made in PyQt5 (taken from Cadence project code)

#### 0.9.2
* 28-OCT-2013: Add 64-bit support and some small fixes

#### 0.9.1
* 15-OCT-2013: Various bug fixes (JH)

#### 0.9.0
* 19-FEB-2011: Nearly complete refactoring of the PipeASIO codebase (asio.c) (JH)

#### 0.8.1
* 05-OCT-2010: Code from Win32 callback thread moved to JACK process callback, except for bufferSwitch() call.
* 05-OCT-2010: Switch from int to float for samples.

#### 0.8.0
* 08-AUG-2010: Forward port JackWASIO changes... needs testing hard. (PLJ)

#### 0.7.6
* 27-DEC-2009: Fixes for compilation on 64-bit platform. (PLJ)

#### 0.7.5
* 29-Oct-2009: Added fork with call to qjackctl from ASIOControlPanel(). (JH)
* 29-Oct-2009: Changed the SCHED_FIFO priority of the win32 callback thread. (JH)
* 28-Oct-2009: Fixed wrongly reported output latency. (JH)

#### 0.7.4
* 08-APR-2008: Updates to the README.TXT (PLJ)
* 02-APR-2008: Move update to "toggle" to hopefully better place (PLJ)
* 24-MCH-2008: Don't trace in win32_callback.  Set patch-level to 4. (PLJ)
* 09-JAN-2008: Nedko Arnaudov supplied a fix for Nuendo under WINE.

#### 0.7.3
* 27-DEC-2007: Make slaving to jack transport work, correct port allocation bug. (RB)

#### 0.7
* 01-DEC-2007: In a fit of insanity, I merged JackLab and Robert Reif code bases. (PLJ)

#### 0.6
* 21-NOV-2007: add dynamic client naming (PLJ)

#### 0.0.3
* 17-NOV-2007: Unique port name code (RR)

#### 0.5
* 03-SEP-2007: port mapping and config file (PLJ)

#### 0.3
* 30-APR-2007: corrected connection of in/outputs (RB)

#### 0.1
* ???????????: Initial RB release (RB)

#### 0.0.2
* 12-SEP-2006: Fix thread bug, tidy up code (RR)

#### 0.0.1
* 31-AUG-2006: Initial version (RR)

### LEGAL STUFF

Copyright (C) 2006 Robert Reif  
Portions copyright (C) 2007 Ralf Beck  
Portions copyright (C) 2007 Johnny Petrantoni  
Portions copyright (C) 2007 Stephane Letz  
Portions copyright (C) 2008 William Steidtmann  
Portions copyright (C) 2010 Peter L Jones  
Portions copyright (C) 2010 Torben Hohn  
Portions copyright (C) 2010 Nedko Arnaudov  
Portions copyright (C) 2011 Christian Schoenebeck  
Portions copyright (C) 2013 Joakim Hernberg  
Portions copyright (C) 2020-2023 Filipe Coelho  

The PipeASIO library code is licensed under LGPL v2.1, see COPYING.LIB for more details.  
The PipeASIO settings UI code is licensed under GPL v2+, see COPYING.GUI for more details.  

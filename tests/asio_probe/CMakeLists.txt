# Standalone Wine PE program that exercises PipeASIO via its COM
# interface so we don't need FL Studio / Proton / Faugus to iterate.

find_program(WINEGCC winegcc REQUIRED)

set(PROBE_SRC  "${CMAKE_CURRENT_SOURCE_DIR}/asio_probe.c")
set(PROBE_OUT  "${CMAKE_CURRENT_BINARY_DIR}/asio_probe.exe.so")
set(PROBE_PE   "${CMAKE_CURRENT_BINARY_DIR}/asio_probe.exe")

add_custom_command(
    OUTPUT  "${PROBE_OUT}"
    COMMAND ${WINEGCC} -m64 -mno-cygwin -mconsole
                       -O0 -g -Wall -Wextra
                       "${PROBE_SRC}"
                       -lole32
                       -o "${PROBE_OUT}"
    DEPENDS "${PROBE_SRC}"
    COMMENT "winegcc asio_probe (Wine PE)"
    VERBATIM
)

add_custom_target(asio_probe ALL DEPENDS "${PROBE_OUT}")

# Copy the runner scripts next to the binary so they ship in build/tests.
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/run.sh"
    "${CMAKE_CURRENT_BINARY_DIR}/run.sh"
    @ONLY
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/gdb.sh"
    "${CMAKE_CURRENT_BINARY_DIR}/gdb.sh"
    @ONLY
)

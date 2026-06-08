/*
 * SettingsDialog.hpp — the PipeASIO settings panel window.
 *
 * Copyright (C) 2026 PipeASIO contributors
 */
#pragma once

#include <QDialog>

#include "PipeWireMonitor.hpp"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class LoadHistogram;
class QSpinBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit SettingsDialog(QWidget *parent = nullptr);

  private slots:
    void onApply();
    void onRestoreDefaults();
    void onMonitorUpdated(const NodeStats &stats);

  private:
    QWidget *buildSettingsTab();
    QWidget *buildMonitorTab();
    void     applyConfig(const struct pipeasio_config &c);
    void     updateLatencyLabel();
    int      currentBufferSize() const;
    int      currentSampleRate() const;

    /* Settings widgets */
    QSpinBox  *m_inputs            = nullptr;
    QSpinBox  *m_outputs           = nullptr;
    QComboBox *m_bufferSize        = nullptr;
    QLabel    *m_latency           = nullptr;
    QComboBox *m_sampleRate        = nullptr;
    QComboBox *m_outputDevice      = nullptr;
    QComboBox *m_inputDevice       = nullptr;
    QCheckBox *m_autoConnect       = nullptr;
    QCheckBox *m_fixedBuffer       = nullptr;
    QCheckBox *m_followDeviceClock = nullptr;
    QLineEdit *m_nodeName          = nullptr;

    /* Monitor widgets */
    QLabel        *m_monQuantum = nullptr;
    QLabel        *m_monRate    = nullptr;
    LoadHistogram *m_monLoad    = nullptr;
    QLabel        *m_monXruns   = nullptr;
    QLabel        *m_monState   = nullptr;

    PipeWireMonitor m_monitor;
};

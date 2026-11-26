#pragma once

#include <QString>
#include <QVector>

namespace ecg {

enum class SourceType { Ring, Krp01 };

enum class SourceState { Disconnected, Connecting, Connected, Running, Error };

struct EcgDeviceDescriptor {
  QString id;             // MAC для BLE, "COM3" для КРП
  QString name;           // "R30 ECG 2C35", "USB-SERIAL CH340"
  QString details;        // RSSI, VID/PID, описание порта
  int signalStrength = 0; // для BLE: RSSI, для COM: 0
};

struct EcgDeviceInfo {
  SourceType type = SourceType::Ring;
  QString id;
  QString name;
  int sampleRateHz = 250;
  int channelCount = 1;
};

struct EcgStreamInfo {
  int sampleRateHz = 250;
  int channelCount = 1;
  bool valuesInMillivolts = false;
};

struct EcgFrame {
  qint64 timestampUs = 0;
  int sampleRateHz = 250;

  QVector<QVector<double>> values;

  bool electrodeDetached = false;
  bool artifact = false;
};

} // namespace ecg

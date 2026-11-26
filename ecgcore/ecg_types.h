#pragma once

#include <QString>
#include <QVector>

namespace ecg {

enum class SourceType {
    Ring,
    Krp01
};

enum class SourceState {
    Disconnected,
    Connecting,
    Connected,
    Running,
    Error
};

struct EcgDeviceDescriptor {
    QString id;
    QString name;
    QString details;
    int signalStrength = 0;
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

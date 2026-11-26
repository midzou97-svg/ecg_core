#pragma once

#include "IEcgSource.h"
#include "RingConnection.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QVector>

namespace ecg {

class RingEcgSource final : public IEcgSource {
    Q_OBJECT

public:
    explicit RingEcgSource(QObject* parent = nullptr);

    QCoro::Task<QVector<EcgDeviceDescriptor>> listDevices() override;
    QCoro::Task<bool> open(const EcgDeviceDescriptor& device) override;
    QCoro::Task<bool> start() override;
    QCoro::Task<bool> stop() override;
    QCoro::Task<bool> close() override;

    SourceState state() const override;
    EcgDeviceInfo deviceInfo() const override;
    EcgStreamInfo streamInfo() const override;

private slots:
    void onConnectionConnected();
    void onConnectionDisconnected();
    void onConnectionError(const QString& message);
    void onRingFrame(const RingProtocol::Frame& frame);
    void onDiscoveryFinished();
    void onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);

private:
    EcgDeviceInfo m_info;
    EcgStreamInfo m_stream;
    EcgDeviceDescriptor m_selectedDevice;

    RingConnection* m_connection = nullptr;
    QBluetoothDeviceDiscoveryAgent* m_discoveryAgent = nullptr;
    QVector<EcgDeviceDescriptor> m_devices;
    bool m_running = false;
};

} // namespace ecg

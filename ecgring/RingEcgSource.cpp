#include "RingEcgSource.h"

#include <QCoro/QCoroSignal>
#include <QBluetoothAddress>
#include <QBluetoothUuid>
#include <QDateTime>
#include <QDebug>

namespace ecg {

RingEcgSource::RingEcgSource(QObject *parent) : IEcgSource(parent) {
  m_info.type = SourceType::Ring;
  m_info.id = QStringLiteral("RING");
  m_info.name = QStringLiteral("ECG Ring");
  m_info.sampleRateHz = 250;
  m_info.channelCount = 1;

  m_stream.sampleRateHz = 250;
  m_stream.channelCount = 1;

  m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

  connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
          this, [this](const QBluetoothDeviceInfo &info) {
            EcgDeviceDescriptor device;
            device.id = info.address().toString();
            device.name = info.name();
            device.details = QStringLiteral("BLE");
            device.signalStrength = info.rssi();
            m_devices.append(device);
          });

  connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this,
          &RingEcgSource::onDiscoveryFinished);

  connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
          this, &RingEcgSource::onDiscoveryError);
}

QCoro::Task<QVector<EcgDeviceDescriptor>> RingEcgSource::listDevices() {
  m_devices.clear();
  m_discoveryAgent->start();

  co_await qCoro(m_discoveryAgent,
                 &QBluetoothDeviceDiscoveryAgent::finished);

  co_return m_devices;
}

QCoro::Task<bool> RingEcgSource::open(const EcgDeviceDescriptor &device) {
  if (m_connection) {
    m_connection->disconnect();
    m_connection->deleteLater();
    m_connection = nullptr;
  }

  m_selectedDevice = device;
  setState(SourceState::Connecting);

  QBluetoothDeviceInfo info(QBluetoothAddress(device.id), device.name, 0);

  m_connection = new RingConnection(this);

  connect(m_connection, &RingConnection::connected, this,
          &RingEcgSource::onConnectionConnected);

  connect(m_connection, &RingConnection::disconnected, this,
          &RingEcgSource::onConnectionDisconnected);

  connect(m_connection, &RingConnection::error, this,
          &RingEcgSource::onConnectionError);

  connect(m_connection, &RingConnection::frameReceived, this,
          &RingEcgSource::onRingFrame);

  m_connection->connectTo(info);

  co_await qCoro(m_connection, &RingConnection::connected);

  setState(SourceState::Connected);
  co_return true;
}

QCoro::Task<bool> RingEcgSource::start() {
  if (!m_connection) {
    emit stateChanged(SourceState::Error);
    co_return false;
  }

  m_running = true;
  setState(SourceState::Running);
  m_connection->startEcg();

  co_return true;
}

QCoro::Task<bool> RingEcgSource::stop() {
  if (!m_connection)
    co_return false;

  m_running = false;
  m_connection->stopEcg();
  setState(SourceState::Connected);

  co_return true;
}

QCoro::Task<bool> RingEcgSource::close() {
  if (m_connection) {
    m_connection->disconnect();
    m_connection->deleteLater();
    m_connection = nullptr;
  }

  m_running = false;
  setState(SourceState::Disconnected);

  co_return true;
}

SourceState RingEcgSource::state() const { return m_state; }

EcgDeviceInfo RingEcgSource::deviceInfo() const { return m_info; }

EcgStreamInfo RingEcgSource::streamInfo() const { return m_stream; }

void RingEcgSource::onConnectionConnected() {
  qDebug() << "Ring connection ready";
}

void RingEcgSource::onConnectionDisconnected() {
  m_running = false;

  if (m_state != SourceState::Disconnected)
    setState(SourceState::Disconnected);
}

void RingEcgSource::onConnectionError(const QString &message) {
  qWarning() << "Ring connection error:" << message;
  setState(SourceState::Error);
}

void RingEcgSource::onRingFrame(const RingProtocol::Frame &frame) {
  if (RingProtocol::dataTypeOf(frame) != 0x0605)
    return;

  const QByteArray &payload = frame.payload;
  if (payload.isEmpty())
    return;

  EcgFrame ecgFrame;
  ecgFrame.timestampUs = QDateTime::currentMSecsSinceEpoch() * 1000;
  ecgFrame.sampleRateHz = m_stream.sampleRateHz;

  ecgFrame.values.resize(1);

  for (qsizetype i = 0; i + 2 < payload.size(); i += 3) {
    const quint32 raw = quint32(quint8(payload.at(i))) |
                        (quint32(quint8(payload.at(i + 1))) << 8) |
                        (quint32(quint8(payload.at(i + 2))) << 16);

    qint32 sample = qint32(raw);

    if (raw & 0x00800000u)
      sample |= qint32(0xFF000000u);

    ecgFrame.values[0].append(double(sample));
  }

  if (!ecgFrame.values[0].isEmpty())
    emit frameReceived(ecgFrame);
}

void RingEcgSource::onDiscoveryFinished() {
  qDebug() << "Ring discovery finished, devices:" << m_devices.size();
}

void RingEcgSource::onDiscoveryError(
    QBluetoothDeviceDiscoveryAgent::Error error) {
  qWarning() << "Ring discovery error:" << int(error);
}

} // namespace ecg

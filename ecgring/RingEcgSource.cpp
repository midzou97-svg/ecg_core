#include "RingEcgSource.h"

#include <QCoro/QCoroTimer>
#include <QDateTime>
#include <QTimer>

namespace ecg {

RingEcgSource::RingEcgSource(QObject *parent) : IEcgSource(parent) {
  m_info.type = SourceType::Ring;
  m_info.id = QStringLiteral("RING-DEMO");
  m_info.name = QStringLiteral("Демо-кольцо");
  m_info.sampleRateHz = 250;
  m_info.channelCount = 1;

  m_stream.sampleRateHz = 250;
  m_stream.channelCount = 1;
  QObject::connect(this, &IEcgSource::frameReceived, [](const EcgFrame &frame) {
    qDebug() << "KRP frame channels:" << frame.values.size()
             << "samples:" << frame.values[0].size();
  });

  QObject::connect(this, &IEcgSource::stateChanged, [](SourceState state) {
    qDebug() << "KRP state:" << static_cast<int>(state);
  });
}

QCoro::Task<QVector<EcgDeviceDescriptor>> RingEcgSource::listDevices() {
  QVector<EcgDeviceDescriptor> result;

  EcgDeviceDescriptor device1;
  device1.id = QStringLiteral("CB:CD:05:DD");
  device1.name = QStringLiteral("Колечко BLE #1");
  device1.details = QStringLiteral("Демо-устройство #1");
  device1.signalStrength = -55;
  result.append(device1);

  EcgDeviceDescriptor device2;
  device2.id = QStringLiteral("B1:FD:04:D1");
  device2.name = QStringLiteral("Колечко BLE #2");
  device2.details = QStringLiteral("Демо-устройство #2");
  device2.signalStrength = -66;
  result.append(device2);

  co_return result;
}

QCoro::Task<bool> RingEcgSource::open(const EcgDeviceDescriptor &device) {
  m_selectedDevice = device;

  setState(SourceState::Connecting);
  co_await QCoro::sleepFor(std::chrono::milliseconds(200));
  setState(SourceState::Connected);

  co_return true;
}

QCoro::Task<bool> RingEcgSource::start() {
  setState(SourceState::Running);

  auto *timer = new QTimer(this);
  timer->setInterval(4);

  connect(timer, &QTimer::timeout, this, [this] {
    EcgFrame frame;
    frame.timestampUs = QDateTime::currentMSecsSinceEpoch() * 1000;
    frame.sampleRateHz = 250;

    frame.values.resize(1);
    frame.values[0].resize(1);
    frame.values[0][0] =
        qSin(QDateTime::currentMSecsSinceEpoch() / 100.0) * 100.0;

    emit frameReceived(frame);
  });

  timer->start();

  co_return true;
}

QCoro::Task<bool> RingEcgSource::stop() {
  setState(SourceState::Connected);
  co_return true;
}

QCoro::Task<bool> RingEcgSource::close() {
  setState(SourceState::Disconnected);
  co_return true;
}

SourceState RingEcgSource::state() const { return m_state; }

EcgDeviceInfo RingEcgSource::deviceInfo() const { return m_info; }

EcgStreamInfo RingEcgSource::streamInfo() const { return m_stream; }

} // namespace ecg

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
}

QCoro::Task<QVector<EcgDeviceDescriptor>> RingEcgSource::listDevices() {
  QVector<EcgDeviceDescriptor> result;
  result.append(EcgDeviceDescriptor("COM1", "КРП01 #1", "Что то еще #1", -55));
  result.append(EcgDeviceDescriptor("COM2", "КРП01 #2", "Что то еще #2", -66));
  co_return result;
}

QCoro::Task<bool> RingEcgSource::open(const ecg::EcgDeviceDescriptor &device) {
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

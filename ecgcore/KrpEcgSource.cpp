#include "KrpEcgSource.h"

#include <QCoro/QCoroTimer>
#include <QDateTime>
#include <QTimer>

namespace ecg {

KrpEcgSource::KrpEcgSource(QObject *parent) : IEcgSource(parent) {
  m_info.type = SourceType::Krp01;
  m_info.id = QStringLiteral("KRP-DEMO");
  m_info.name = QStringLiteral("Демо-КРП");
  m_info.sampleRateHz = 500;
  m_info.channelCount = 8;

  m_stream.sampleRateHz = 500;
  m_stream.channelCount = 8;
}
QCoro::Task<QVector<EcgDeviceDescriptor>> KrpEcgSource::listDevices() {
  QVector<EcgDeviceDescriptor> result;
  result.append(EcgDeviceDescriptor("CB:CD:05:DD", "Колечко BLE #1",
                                    "Что то еще #1", -55));
  result.append(EcgDeviceDescriptor("B1:FD:04:D1", "Колечко BLE #2",
                                    "Что то еще #2", -66));
  co_return result;
}

QCoro::Task<bool> KrpEcgSource::open(const ecg::EcgDeviceDescriptor &device) {
  m_selectedDevice = device;
  setState(SourceState::Connecting);
  co_await QCoro::sleepFor(std::chrono::milliseconds(200));
  setState(SourceState::Connected);
  co_return true;
}

QCoro::Task<bool> KrpEcgSource::start() {
  setState(SourceState::Running);

  auto *timer = new QTimer(this);
  timer->setInterval(2);

  connect(timer, &QTimer::timeout, this, [this] {
    EcgFrame frame;
    frame.timestampUs = QDateTime::currentMSecsSinceEpoch() * 1000;
    frame.sampleRateHz = 500;

    frame.values.resize(8);

    for (int ch = 0; ch < 8; ++ch) {
      frame.values[ch].resize(1);
      frame.values[ch][0] =
          qSin(QDateTime::currentMSecsSinceEpoch() / 100.0 + ch) * 100.0;
    }

    emit frameReceived(frame);
  });

  timer->start();

  co_return true;
}

QCoro::Task<bool> KrpEcgSource::stop() {
  setState(SourceState::Connected);
  co_return true;
}

QCoro::Task<bool> KrpEcgSource::close() {
  setState(SourceState::Disconnected);
  co_return true;
}

SourceState KrpEcgSource::state() const { return m_state; }

EcgDeviceInfo KrpEcgSource::deviceInfo() const { return m_info; }

EcgStreamInfo KrpEcgSource::streamInfo() const { return m_stream; }

} // namespace ecg

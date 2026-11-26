#include <QCoreApplication>
#include <QCoro/QCoroTask>
#include <QCoro/QCoroTimer>
#include <QDebug>

#include "EcgSourceFactory.h"
#include "IEcgSource.h"
#include "ecg_types.h"

using namespace ecg;

QCoro::Task<> runRing() {
  auto *source = EcgSourceFactory::create(SourceType::Ring, nullptr);

  if (!source) {
    qWarning() << "Ring source not available";
    co_return;
  }

  QObject::connect(source, &IEcgSource::frameReceived,
                   [](const EcgFrame &frame) {
                     qDebug() << "RING frame channels:" << frame.values.size()
                              << "samples:" << frame.values[0].size();
                   });

  QObject::connect(source, &IEcgSource::stateChanged, [](SourceState state) {
    qDebug() << "RING state:" << static_cast<int>(state);
  });

  const auto listDescr = co_await source->listDevices();

  const bool opened = co_await source->open(listDescr.first());

  if (!opened) {
    qWarning() << "RING open failed";
    source->deleteLater();
    co_return;
  }

  const bool started = co_await source->start();

  if (!started) {
    qWarning() << "RING start failed";
    co_await source->close();
    source->deleteLater();
    co_return;
  }

  co_await QCoro::sleepFor(std::chrono::seconds(2));

  co_await source->stop();
  co_await source->close();

  source->deleteLater();
}

QCoro::Task<> runKrp() {
  auto *source = EcgSourceFactory::create(SourceType::Krp01, nullptr);

  if (!source) {
    qWarning() << "KRP source not available";
    co_return;
  }

  QObject::connect(source, &IEcgSource::frameReceived,
                   [](const EcgFrame &frame) {
                     qDebug() << "KRP frame channels:" << frame.values.size()
                              << "samples:" << frame.values[0].size();
                   });

  QObject::connect(source, &IEcgSource::stateChanged, [](SourceState state) {
    qDebug() << "KRP state:" << static_cast<int>(state);
  });

  const auto listDescr = co_await source->listDevices();

  const bool opened = co_await source->open(listDescr.first());

  if (!opened) {
    qWarning() << "KRP open failed";
    source->deleteLater();
    co_return;
  }

  const bool started = co_await source->start();

  if (!started) {
    qWarning() << "KRP start failed";
    co_await source->close();
    source->deleteLater();
    co_return;
  }

  co_await QCoro::sleepFor(std::chrono::seconds(2));

  co_await source->stop();
  co_await source->close();

  source->deleteLater();
}

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  runRing();
  // runKrp();

  return app.exec();
}
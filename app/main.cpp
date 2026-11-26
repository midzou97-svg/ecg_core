#include <QCoreApplication>
#include <QCoro/QCoroTask>
#include <QCoro/QCoroTimer>
#include <QDebug>

#include "IEcgSource.h"
#include "KrpSourceApi.h"
#include "RingSourceApi.h"

using namespace ecg;

QCoro::Task<> runRing() {
  auto *source = ecg::ring::createSource(nullptr);

  const auto devices = co_await source->listDevices();

  if (devices.isEmpty()) {
    qWarning() << "No ring devices";
    co_return;
  }

  const EcgDeviceDescriptor device = devices.first();

  co_await source->open(device);
  co_await source->start();

  co_await QCoro::sleepFor(std::chrono::seconds(2));

  co_await source->stop();
  co_await source->close();

  source->deleteLater();
}

QCoro::Task<> runKrp() {
  auto *source = ecg::krp::createSource(nullptr);

  const auto devices = co_await source->listDevices();

  if (devices.isEmpty()) {
    qWarning() << "No KRP devices";
    co_return;
  }

  const EcgDeviceDescriptor device = devices.first();

  co_await source->open(device);
  co_await source->start();

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
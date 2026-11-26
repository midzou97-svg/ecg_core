#include <QCoreApplication>
#include <QDebug>
#include <QCoro/QCoroTask>
#include <QCoro/QCoroTimer>

#include "RingSourceApi.h"
#include "KrpSourceApi.h"
#include "IEcgSource.h"

using namespace ecg;

QCoro::Task<> runRing()
{
    auto* source = ecg::ring::createSource(nullptr);

    const auto devices = co_await source->listDevices();

    if (devices.isEmpty()) {
        qWarning() << "No ring devices";
        co_return;
    }

    const EcgDeviceDescriptor device = devices.first();

    QObject::connect(source, &IEcgSource::frameReceived,
        [](const EcgFrame& frame) {
            qDebug() << "RING frame channels:" << frame.values.size()
                     << "samples:" << frame.values[0].size();
        });

    QObject::connect(source, &IEcgSource::stateChanged,
        [](SourceState state) {
            qDebug() << "RING state:" << static_cast<int>(state);
        });

    co_await source->open(device);
    co_await source->start();

    co_await QCoro::sleepFor(std::chrono::seconds(2));

    co_await source->stop();
    co_await source->close();

    source->deleteLater();
}

QCoro::Task<> runKrp()
{
    auto* source = ecg::krp::createSource(nullptr);

    const auto devices = co_await source->listDevices();

    if (devices.isEmpty()) {
        qWarning() << "No KRP devices";
        co_return;
    }

    const EcgDeviceDescriptor device = devices.first();

    QObject::connect(source, &IEcgSource::frameReceived,
        [](const EcgFrame& frame) {
            qDebug() << "KRP frame channels:" << frame.values.size()
                     << "samples:" << frame.values[0].size();
        });

    QObject::connect(source, &IEcgSource::stateChanged,
        [](SourceState state) {
            qDebug() << "KRP state:" << static_cast<int>(state);
        });

    co_await source->open(device);
    co_await source->start();

    co_await QCoro::sleepFor(std::chrono::seconds(2));

    co_await source->stop();
    co_await source->close();

    source->deleteLater();
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    runRing();
    // runKrp();

    return app.exec();
}

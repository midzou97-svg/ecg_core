#pragma once

#include "EcgCore_global.h"
#include "ecg_types.h"
#include <QCoro/QCoroTask>
#include <QObject>
#include <QString>
#include <QVector>

namespace ecg {

class ECGCORE_EXPORT IEcgDeviceDiscovery : public QObject {
  Q_OBJECT

public:
  explicit IEcgDeviceDiscovery(QObject *parent = nullptr) : QObject(parent) {}

  virtual ~IEcgDeviceDiscovery() = default;

  // Запустить поиск устройств.
  virtual QCoro::Task<bool> startDiscovery() = 0;

  // Остановить поиск.
  virtual QCoro::Task<bool> stopDiscovery() = 0;

  virtual bool isDiscovering() const = 0;

signals:
  void deviceFound(const ecg::EcgDeviceDescriptor &device);
  void discoveryFinished();
  void errorOccurred(int code, const QString &message);
};

} // namespace ecg
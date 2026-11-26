#pragma once

#include "EcgCore_global.h"
#include "ecg_types.h"

#include <QCoro/QCoroTask>
#include <QObject>

namespace ecg {

class ECGCORE_EXPORT IEcgSource : public QObject {
  Q_OBJECT

public:
  explicit IEcgSource(QObject *parent = nullptr) : QObject(parent) {}

  virtual ~IEcgSource() = default;

  virtual QCoro::Task<QVector<EcgDeviceDescriptor>> listDevices() = 0;
  virtual QCoro::Task<bool> open(const EcgDeviceDescriptor &device) = 0;
  virtual QCoro::Task<bool> start() = 0;
  virtual QCoro::Task<bool> stop() = 0;
  virtual QCoro::Task<bool> close() = 0;

  virtual SourceState state() const = 0;
  virtual EcgDeviceInfo deviceInfo() const = 0;
  virtual EcgStreamInfo streamInfo() const = 0;

signals:
  void frameReceived(const ecg::EcgFrame &frame);
  void stateChanged(ecg::SourceState state);

protected:
  void setState(SourceState state) {
    if (m_state == state)
      return;

    m_state = state;
    emit stateChanged(m_state);
  }

  SourceState m_state = SourceState::Disconnected;
};

} // namespace ecg

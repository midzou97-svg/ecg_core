#pragma once

#include "IEcgSource.h"

namespace ecg {

class KrpEcgSource final : public IEcgSource {
  Q_OBJECT

public:
  explicit KrpEcgSource(QObject *parent = nullptr);

  QCoro::Task<QVector<EcgDeviceDescriptor>> listDevices() override;

  QCoro::Task<bool> open(const ecg::EcgDeviceDescriptor &device) override;
  QCoro::Task<bool> start() override;
  QCoro::Task<bool> stop() override;
  QCoro::Task<bool> close() override;

  SourceState state() const override;
  EcgDeviceInfo deviceInfo() const override;
  EcgStreamInfo streamInfo() const override;

private:
  EcgDeviceInfo m_info;
  EcgStreamInfo m_stream;
  ecg::EcgDeviceDescriptor m_selectedDevice;
};

} // namespace ecg

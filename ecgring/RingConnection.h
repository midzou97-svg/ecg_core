#pragma once
#include "RingProtocol.h"
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QObject>

class RingConnection : public QObject {
  Q_OBJECT
public:
  explicit RingConnection(QObject *parent = nullptr);

  static constexpr const wchar_t *ServiceUuid =
      L"be940000-7333-be46-b7ae-689e71722bd5";
  static constexpr const wchar_t *WriteUuid =
      L"be940001-7333-be46-b7ae-689e71722bd5"; // C_1
  static constexpr const wchar_t *NotifyUuid =
      L"be940003-7333-be46-b7ae-689e71722bd5"; // C_3

  void connectTo(const QBluetoothDeviceInfo &deviceInfo);
  void sendCommand(quint16 dataType, const QByteArray &payload = {});
  void disconnect();
  void startEcg();
  void stopEcg();

signals:
  void connected();
  void disconnected();
  void frameReceived(const RingProtocol::Frame &frame);
  void error(const QString &message);
  void logMessage(const QString &message);
  void ecgSampleReceived(int sample);

private slots:
  void onConnected();
  void onServiceDiscovered(const QBluetoothUuid &uuid);
  void onDiscoveryFinished();
  void onServiceStateChanged(QLowEnergyService::ServiceState s);
  void onCharacteristicChanged(const QLowEnergyCharacteristic &c,
                               const QByteArray &value);

private:
  void enableIndications();
  void sendFrame(const QByteArray &frame);
  void runInitSequence();
  void processFrame(const RingProtocol::Frame &frame);
  void processEcgPayload(const QByteArray &p);

private:
  QLowEnergyController *m_controller = nullptr;
  QLowEnergyService *m_service = nullptr;
  QLowEnergyCharacteristic m_writeChar;  // C_1
  QLowEnergyCharacteristic m_notifyChar; // C_3
  QByteArray m_rxBuffer;
  bool m_ready = false;

  // Цепочка запуска ЭКГ: 1..4 - шаги, 5 - поток идёт, -1 - неактивна
  int m_ecgChainStep = -1;
  quint32 m_ecgSampleCount = 0;
};

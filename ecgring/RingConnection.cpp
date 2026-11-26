#include "RingConnection.h"
#include <QBluetoothUuid>
#include <QDateTime>
#include <QDebug>
#include <QLowEnergyDescriptor>

RingConnection::RingConnection(QObject *parent) : QObject(parent) {}

void RingConnection::connectTo(const QBluetoothDeviceInfo &deviceInfo) {
  m_controller = QLowEnergyController::createCentral(deviceInfo, this);
  connect(m_controller, &QLowEnergyController::connected, this,
          &RingConnection::onConnected);
  connect(m_controller, &QLowEnergyController::disconnected, this,
          &RingConnection::disconnected);
  connect(m_controller, &QLowEnergyController::serviceDiscovered, this,
          &RingConnection::onServiceDiscovered);
  connect(m_controller, &QLowEnergyController::discoveryFinished, this,
          &RingConnection::onDiscoveryFinished);
  connect(m_controller, &QLowEnergyController::errorOccurred, this,
          [this](QLowEnergyController::Error e) {
            emit error(QString("BLE error: %1").arg(int(e)));
          });
  m_controller->connectToDevice();
}

void RingConnection::onConnected() { m_controller->discoverServices(); }

void RingConnection::onServiceDiscovered(const QBluetoothUuid &uuid) {
  Q_UNUSED(uuid);
}

void RingConnection::onDiscoveryFinished() {
  const QBluetoothUuid serviceUuid(QString::fromWCharArray(ServiceUuid));

  m_service = m_controller->createServiceObject(serviceUuid, this);

  if (!m_service) {
    emit error("Основной сервис кольца не найден");
    return;
  }

  connect(m_service, &QLowEnergyService::stateChanged, this,
          &RingConnection::onServiceStateChanged);

  connect(m_service, &QLowEnergyService::characteristicChanged, this,
          &RingConnection::onCharacteristicChanged);

  connect(m_service, &QLowEnergyService::errorOccurred, this,
          [this](QLowEnergyService::ServiceError code) {
            emit error(QString("Ошибка BLE-сервиса: %1").arg(int(code)));
          });

  connect(
      m_service, &QLowEnergyService::descriptorWritten, this,
      [this](const QLowEnergyDescriptor &descriptor, const QByteArray &value) {
        qDebug().noquote() << "CCCD written:" << descriptor.uuid().toString()
                           << value.toHex(' '); 

        if (value != QByteArray::fromHex("0200"))
          return;

        const auto d3 = m_notifyChar.clientCharacteristicConfiguration();

        const auto d1 = m_writeChar.clientCharacteristicConfiguration();

        if (d3.isValid() && descriptor == d3) {
          if (!d1.isValid()) {
            emit error("CCCD характеристики C_1 не найден");
            return;
          }

          m_service->writeDescriptor(d1, QByteArray::fromHex("0200"));

        } else if (d1.isValid() && descriptor == d1 && !m_ready) {
          m_ready = true;
          runInitSequence(); // установит время и запросит инфо
          sendCommand(0x0200, QByteArray("GC"));
          emit connected();
        }
      });

  m_service->discoverDetails();
}

void RingConnection::onServiceStateChanged(QLowEnergyService::ServiceState s) {
  if (s != QLowEnergyService::ServiceDiscovered)
    return;

  for (const QLowEnergyCharacteristic &c : m_service->characteristics()) {
    if (c.uuid() == QBluetoothUuid(QString::fromWCharArray(WriteUuid)))
      m_writeChar = c;
    if (c.uuid() == QBluetoothUuid(QString::fromWCharArray(NotifyUuid)))
      m_notifyChar = c;
  }
  if (m_writeChar.isValid() && m_notifyChar.isValid())
    enableIndications();
  else
    emit error("Характеристики C_1/C_3 не найдены");
}

// КРИТИЧНО: пишем 02 00 (indication), НЕ 01 00 (notification),
// иначе кольцо разрывает соединение (~35 мс). Сначала C_3, затем C_1.
void RingConnection::enableIndications() {
  m_ready = false;
  m_rxBuffer.clear();

  const auto d3 = m_notifyChar.clientCharacteristicConfiguration();

  if (!d3.isValid()) {
    emit error("CCCD характеристики C_3 не найден");
    return;
  }

  m_service->writeDescriptor(d3, QByteArray::fromHex("0200"));
}

void RingConnection::runInitSequence() {
  // Последовательность из протокола: время -> флаги -> схема чипа -> инфо
  QDateTime now = QDateTime::currentDateTime();
  QByteArray t;
  quint16 year = quint16(now.date().year());
  t.append(char(year & 0xFF)).append(char(year >> 8));
  t.append(char(now.date().month()))
      .append(char(now.date().day()))
      .append(char(now.time().hour()))
      .append(char(now.time().minute()))
      .append(char(now.time().second()))
      .append(char(now.date().dayOfWeek() - 1)); // 0 = понедельник
  sendCommand(0x0100, t);

  sendCommand(0x0201, "GF"); // feature flags
  sendCommand(0x021B);       // схема чипа
  sendCommand(0x0200, "GC"); // информация об устройстве
}

void RingConnection::sendCommand(quint16 dataType, const QByteArray &payload) {
  sendFrame(RingProtocol::buildCommand(dataType, payload));
}

void RingConnection::sendFrame(const QByteArray &frame) {
  if (!m_service || !m_writeChar.isValid() || !m_ready) {
    emit error("Нельзя отправить команду: BLE-канал не готов");
    return;
  }

  emit logMessage("BLE TX: " + QString::fromLatin1(frame.toHex(' ')));

  m_service->writeCharacteristic(m_writeChar, frame,
                                 QLowEnergyService::WriteWithoutResponse);
}

void RingConnection::onCharacteristicChanged(const QLowEnergyCharacteristic &c,
                                             const QByteArray &value) {
  if (c.uuid() != m_notifyChar.uuid() && c.uuid() != m_writeChar.uuid()) {
    return;
  }

  emit logMessage(QString("BLE RX [%1]: %2")
                      .arg(c.uuid().toString())
                      .arg(QString::fromLatin1(value.toHex(' '))));

  m_rxBuffer.append(value);

  QVector<RingProtocol::Frame> frames;
  const int consumed = RingProtocol::parseFrames(m_rxBuffer, frames);

  if (consumed > 0)
    m_rxBuffer.remove(0, consumed);

  for (const auto &frame : frames) {
    emit frameReceived(frame);
    processFrame(frame);
  }
}

void RingConnection::disconnect() {
  if (m_controller)
    m_controller->disconnectFromDevice();
}

void RingConnection::processEcgPayload(const QByteArray &p) {
  // 24-битные знаковые отсчёты, little-endian, по 3 байта
  // (формат из YCBT SDK: AIPraseDataUtil.praseData).
  qsizetype count = p.size() / 3;

  for (qsizetype i = 0; i + 2 < p.size(); i += 3) {
    quint32 raw = quint32(quint8(p.at(i))) |
                  (quint32(quint8(p.at(i + 1))) << 8) |
                  (quint32(quint8(p.at(i + 2))) << 16);

    qint32 sample = qint32(raw);
    if (raw & 0x00800000u)
      sample |= qint32(0xFF000000u); // знаковое расширение

    ++m_ecgSampleCount;
    emit ecgSampleReceived(sample);

    // Первые отсчёты кадра показываем полностью, дальше - кратко.
    if (m_ecgSampleCount <= 20 || m_ecgSampleCount % 100 == 0) {
      emit logMessage(
          QString("  ECG отсчёт #%1: %2").arg(m_ecgSampleCount).arg(sample));
    }
  }

  if (p.size() % 3 != 0) {
    emit logMessage(QString("  ECG: лишние %1 байт в кадре").arg(p.size() % 3));
  }

  emit logMessage(QString("  ECG: кадр 0x0605, %1 отсчётов (всего %2)")
                      .arg(count)
                      .arg(m_ecgSampleCount));
}

void RingConnection::processFrame(const RingProtocol::Frame &frame) {
  const quint16 type = RingProtocol::dataTypeOf(frame);
  const QByteArray &p = frame.payload;

  const auto byte = [&p](qsizetype offset) -> quint8 {
    return quint8(p.at(offset));
  };

  const auto hex = [](const QByteArray &data) {
    return QString::fromLatin1(data.toHex(' '));
  };

  emit logMessage(QString("RX 0x%1 | %2 байт | %3")
                      .arg(type, 4, 16, QLatin1Char('0'))
                      .arg(p.size())
                      .arg(hex(p)));

  // Поток ЭКГ обрабатываем до проверки ошибок: это многобайтовый кадр.
  if (type == 0x0605) {
    processEcgPayload(p);
    return;
  }

  // Однобайтовые ошибки протокола.
  if (p.size() == 1 && byte(0) >= 0xF0) {
    QString reason;

    switch (byte(0)) {
    case 0xFB:
      reason = "Команда не поддерживается";
      break;
    case 0xFC:
      reason = "Ключ команды не поддерживается";
      break;
    case 0xFD:
      reason = "Неверная длина";
      break;
    case 0xFE:
      reason = "Неверные данные";
      break;
    case 0xFF:
      reason = "Неверная CRC";
      break;
    default:
      reason = "Неизвестный код ошибки";
      break;
    }

    emit logMessage("  Ответ устройства: " + reason);

    // После отказа запроса возможностей всё равно
    // пробуем прочитать схему чипа.
    if (type == 0x0201)
      sendCommand(0x021B);

    return;
  }

  switch (type) {
  case 0x0314: {
    // AppECGPPGStatus: статус контакта электродов.
    if (p.size() >= 1) {
      const int ecgStatus = int(byte(0));
      QString status;
      if (ecgStatus == 0)
        status = "контакт хороший";
      else if (ecgStatus == 1)
        status = "электрод отсоединён";
      else if (ecgStatus == 2)
        status = "контакт потерян";
      else
        status = "неизвестный статус";
      emit logMessage(
          QString("  Статус ЭКГ (0x0314): %1 (%2)").arg(ecgStatus).arg(status));
    }
    break;
  }
  case 0x0200: {
    if (p.size() < 8) {
      emit logMessage("  Недостаточно данных для информации об устройстве");
      return;
    }

    const quint16 deviceId = quint16(byte(0)) | (quint16(byte(1)) << 8);

    emit logMessage(QString("  Device ID: %1 (0x%2)")
                        .arg(deviceId)
                        .arg(deviceId, 4, 16, QLatin1Char('0')));

    emit logMessage(QString("  Прошивка по схеме документа: %1.%2")
                        .arg(int(byte(3)))
                        .arg(int(byte(2))));

    emit logMessage(QString("  Состояние батареи, код: %1").arg(int(byte(4))));

    emit logMessage(
        QString("  Уровень батареи, исходное значение: %1").arg(int(byte(5))));

    emit logMessage(QString("  Состояние привязки, код: %1").arg(int(byte(6))));

    emit logMessage(
        QString("  Состояние синхронизации, код: %1").arg(int(byte(7))));

    if (p.size() >= 24) {
      emit logMessage(
          QString("  Hardware type по схеме документа: %1").arg(int(byte(18))));
    }

    if (p.size() > 8) {
      emit logMessage("  Дополнительные поля [8..]: " + hex(p.mid(8)));
    }

    // Следующий диагностический запрос.
    sendCommand(0x0201, QByteArray("GF"));
    break;
  }

  case 0x0201: {
    emit logMessage("  Возможности устройства: получен массив флагов");

    // Пока нет проверенной карты функций:
    // показываем каждый байт и установленные биты.
    for (qsizetype offset = 0; offset < p.size(); ++offset) {
      const quint8 value = byte(offset);

      QString bits;
      for (int bit = 0; bit < 8; ++bit) {
        if (value & (1u << bit)) {
          if (!bits.isEmpty())
            bits += ", ";

          bits += QString::number(bit);
        }
      }

      if (bits.isEmpty())
        bits = "нет";

      emit logMessage(QString("    [%1] 0x%2 | биты LSB=0: %3")
                          .arg(offset)
                          .arg(int(value), 2, 16, QLatin1Char('0'))
                          .arg(bits));
    }

    sendCommand(0x021B);
    break;
  }

  case 0x021B: {
    if (p.isEmpty()) {
      emit logMessage("  Пустой ответ со схемой чипа");
      return;
    }

    const int scheme = byte(0);
    const bool jieLi = scheme == 3 || scheme == 4 || scheme == 5;

    emit logMessage(QString("  Схема чипа: %1 (%2)")
                        .arg(scheme)
                        .arg(jieLi ? "JieLi по классификации SDK"
                                   : "другая / пока не расшифрована"));

    emit logMessage("Диагностический опрос при подключении завершён");
    break;
  }

  case 0x0302: {
    // AppBloodSwitch: управление ЭКГ.
    if (m_ecgChainStep == 1) {
      m_ecgChainStep = 2;
      emit logMessage(
          "  ЭКГ: шаг 2 - открытие волнового канала (0x030B 01 01)");
      sendCommand(0x030B, QByteArray("\x01\x01", 2));
    } else if (m_ecgChainStep == -1) {
      emit logMessage("  ЭКГ: остановка подтверждена кольцом");
    } else {
      emit logMessage(QString("  ЭКГ: ответ 0x0302, payload=0x%1 (шаг %2)")
                          .arg(int(byte(0)), 2, 16, QLatin1Char('0'))
                          .arg(m_ecgChainStep));
    }
    break;
  }

  case 0x0309: {
    // AppControlReal: после его подтверждения отправляем финальную 0x030B.
    if (m_ecgChainStep == 3) {
      m_ecgChainStep = 4;
      emit logMessage(
          "  ЭКГ: шаг 4 - финальное включение канала (0x030B 01 00)");
      sendCommand(0x030B, QByteArray("\x01\x00", 2));
    } else {
      emit logMessage("  ЭКГ: подтверждение 0x0309 (AppControlReal)");
    }
    break;
  }

  case 0x030B: {
    if (m_ecgChainStep == 2) {
      m_ecgChainStep = 3;
      emit logMessage(
          "  ЭКГ: шаг 3 - включение выгрузки данных (0x0309 01 03 02)");
      sendCommand(0x0309, QByteArray("\x01\x03\x02", 3));
    } else if (m_ecgChainStep == 4) {
      m_ecgChainStep = 5;
      m_ecgSampleCount = 0;
      emit logMessage(
          "  ЭКГ: цепочка завершена, запрашиваю режим ЭКГ (0x0227)");
      sendCommand(0x0227);
    } else {
      emit logMessage("  Ответ 0x030B (AppControlWave)");
    }
    break;
  }

  case 0x040B: {
    // DeviceEndECG: кольцо сообщило о завершении ЭКГ.
    emit logMessage("  ЭКГ: кольцо сообщило о завершении измерения (0x040B)");
    m_ecgChainStep = -1;
    break;
  }

  default:
    emit logMessage("  Расшифровка этого типа пока не добавлена");
    break;
  }
}

void RingConnection::startEcg() {
  if (!m_ready) {
    emit error("ЭКГ: соединение не готово");
    return;
  }

  // 1. Синхронизация времени (0x0100) — как в SDK при подключении
  QDateTime now = QDateTime::currentDateTime();
  QByteArray t;
  quint16 year = quint16(now.date().year());
  t.append(char(year & 0xFF)).append(char(year >> 8));
  t.append(char(now.date().month())).append(char(now.date().day()));
  t.append(char(now.time().hour())).append(char(now.time().minute()));
  t.append(char(now.time().second()));
  t.append(char(now.date().dayOfWeek() - 1));
  sendCommand(0x0100, t);

  // 2. Рука по умолчанию: левая (0x0108 SettingHandWear), правая 106
  sendCommand(0x0108, QByteArray(1, char(0x01)));

  m_ecgSampleCount = 0;
  m_ecgChainStep = 1;
  emit logMessage("ЭКГ: время и рука установлены, шаг 1 (0x0302 payload=02)");
  sendCommand(0x0302, QByteArray(1, char(0x02)));
}

void RingConnection::stopEcg() {
  if (!m_ready) {
    emit error("ЭКГ: соединение не готово");
    return;
  }

  m_ecgChainStep = -1;
  emit logMessage("ЭКГ: остановка, команда 0x0302 payload=00");
  sendCommand(0x0302, QByteArray(1, char(0x00)));
}

#pragma once
#include <QByteArray>
#include <QVector>

// Формирование и разбор кадров протокола кольца Yucheng
class RingProtocol {
public:
  // Группы команд (byte 0 кадра)
  enum Group : quint8 {
    GroupSetting = 1,
    GroupGet = 2,
    GroupAppControl = 3,
    GroupDevControl = 4,
    GroupHealth = 5,
    GroupReal = 6,
  };

  struct Frame {
    quint8 group = 0;
    quint8 key = 0;
    QByteArray payload;
  };

  // CRC-16/CCITT-FALSE (init 0xFFFF, poly 0x1021), младший байт вперёд
  static quint16 crc16(const QByteArray &data);

  // Собрать полный кадр для отправки (group, key, payload)
  static QByteArray buildFrame(quint8 group, quint8 key,
                               const QByteArray &payload = {});

  // Удобные обёртки: dataType = (group << 8) | key
  static QByteArray buildCommand(quint16 dataType,
                                 const QByteArray &payload = {});

  // Разобрать входящий буфер: извлекает все полные кадры,
  // возвращает число съеденных байт; кадры кладёт в out
  static int parseFrames(const QByteArray &buffer, QVector<Frame> &out);

  static quint16 dataTypeOf(const Frame &f) {
    return (quint16(f.group) << 8) | f.key;
  }
};

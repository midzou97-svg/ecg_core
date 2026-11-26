#include "RingProtocol.h"

quint16 RingProtocol::crc16(const QByteArray &data) {
  quint16 crc = 0xFFFF;

  for (char ch : data) {
    crc ^= quint16(quint8(ch)) << 8;

    for (int bit = 0; bit < 8; ++bit) {
      if (crc & 0x8000)
        crc = quint16((crc << 1) ^ 0x1021);
      else
        crc = quint16(crc << 1);
    }
  }

  return crc;
}

QByteArray RingProtocol::buildFrame(quint8 group, quint8 key,
                                    const QByteArray &payload) {
  QByteArray frame;

  // Полная длина: заголовок + данные + CRC.
  const quint16 length = quint16(6 + payload.size());

  frame.append(char(group));
  frame.append(char(key));
  frame.append(char(length & 0xFF));
  frame.append(char((length >> 8) & 0xFF));
  frame.append(payload);

  const quint16 crc = crc16(frame);

  frame.append(char(crc & 0xFF));
  frame.append(char((crc >> 8) & 0xFF));

  return frame;
}

QByteArray RingProtocol::buildCommand(quint16 dataType,
                                      const QByteArray &payload) {
  return buildFrame(quint8(dataType >> 8), quint8(dataType & 0xFF), payload);
}

int RingProtocol::parseFrames(const QByteArray &buffer, QVector<Frame> &out) {
  int pos = 0;
  while (buffer.size() - pos >= 6) { // минимум: 4 заголовка + 2 CRC
    quint8 group = quint8(buffer[pos]);
    quint8 key = quint8(buffer[pos + 1]);
    quint16 len = quint16(quint8(buffer[pos + 2])) |
                  (quint16(quint8(buffer[pos + 3])) << 8);
    if (len < 6 || pos + len > buffer.size())
      break; // ждём остальные куски кадра
    Frame f;
    f.group = group;
    f.key = key;
    f.payload = buffer.mid(pos + 4, len - 6);
    // при желании проверить CRC:
    // quint16 crc = crc16(buffer.mid(pos, len - 2));
    out.append(f);
    pos += len;
  }
  return pos;
}

#ifndef ARDUINO_HPP
#define ARDUINO_HPP

#include <QtSerialPort/QSerialPort>

constexpr uint8_t batt_temp_cmd = 0x00;
constexpr uint8_t batt_volt_cmd = 0x01;
constexpr uint8_t volt1_cmd = 0x02;
constexpr uint8_t volt2_cmd = 0x03;
constexpr uint8_t tuketim_cmd = 0x04;
constexpr uint8_t speed_cmd = 0x05;

constexpr uint8_t cmds[] = {
  batt_temp_cmd,
  batt_volt_cmd,
  volt1_cmd,
  volt2_cmd,
  tuketim_cmd,
  speed_cmd
};

struct Msg{
  uint8_t len;
  uint8_t buff[256];
  uint16_t crc;

  Msg clone();
};

class Arduino : public QObject
{
  Q_OBJECT
public:
  explicit Arduino();
  void send_msg(uint8_t *datas, uint8_t len);
  void send_only_cmd(uint8_t cmd);
  uint8_t get_cmd(Msg &msg);
  uint16_t get_uint16(Msg &msg, uint8_t offset);
  uint32_t get_uint32(Msg &msg, uint8_t offset);

private:
  QSerialPort ser;

  void get_byte(uint8_t byte);
  bool crc_check();

signals:
  msg_complete(Msg msg);

public slots:
  void wait_read();
};

#endif // ARDUINO_HPP

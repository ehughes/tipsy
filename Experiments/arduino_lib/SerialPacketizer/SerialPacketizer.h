#ifndef _SERIALPACKETIZER_H
#define _SERIALPACKETIZER_H

/*
  SerialPacketizer:

  simple packet framing/checksum for textual data packets over serial lines
*/

#include <Arduino.h>
#include <functional>

#define START_BYTE 0x1D // group separator
#define END_BYTE 0x00 // null
#define BUFFER_SIZE 512

class SerialPacketizer {
public:
  SerialPacketizer(HardwareSerial *s);
  void process(void);
  void onEvent(std::function<void (uint8_t mode, uint8_t *payload, size_t length)> eventHandler);
  void send(uint8_t mode, uint8_t *payload, size_t length);

protected:
  enum state_t { GET_START, GET_MODE, GET_DATA, GET_CHECKSUM };
  state_t state;
  HardwareSerial *serial;
  uint8_t checksum;
  uint8_t mode;
  uint8_t serialBuffer[BUFFER_SIZE];
  uint pos;
  std::function<void (uint8_t mode, uint8_t *payload, size_t length)> _eventHandler;
};

#endif
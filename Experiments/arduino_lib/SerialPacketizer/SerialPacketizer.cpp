#include "SerialPacketizer.h"

//#define SERIAL_DEBUG

SerialPacketizer::SerialPacketizer(HardwareSerial *s/*, SerialPacketObserver *spo*/) :
  state(GET_START), serial(s)//, observer(spo)
{

}

void SerialPacketizer::process(void) {
  while(serial->available()) {
    uint8_t currentByte = serial->read();
#ifdef SERIAL_DEBUG
    //Serial.printf("sp %X %c %d %d", currentByte, currentByte, mode, pos);
    //Serial.println();
#endif
    if (currentByte == START_BYTE) {
#ifdef SERIAL_DEBUG
      if (state != GET_START) {
        Serial.println("START_BYTE found before packet was properly terminated");
        Serial.write(serialBuffer, pos);
        Serial.println();
      }
#endif
      checksum = 0;
      pos = 0;
      state = GET_MODE;
      continue;
    }
    switch (state) {
      case GET_MODE:
      mode = currentByte;
      state = GET_DATA;
      break;

      case GET_DATA:
      if (currentByte == END_BYTE) {
        state = GET_CHECKSUM;
        break;
      } 
      // calculate checksum
      checksum = checksum ^ currentByte;
      // store byte
      if (pos >= BUFFER_SIZE - 1) {
        // buffer size exceeded
#ifdef SERIAL_DEBUG
        Serial.println("Buffer size exceeded");
        Serial.write(serialBuffer, pos);
        Serial.println();
#endif
        state = GET_START;
        break;
      }
      serialBuffer[pos] = currentByte;
      pos++;
      break;

      case GET_CHECKSUM:
      state = GET_START;
      // compare checksum
      if (checksum != currentByte) {
        // invalid checksum, discard data
#ifdef SERIAL_DEBUG
        Serial.println("Invalid checksum");
        Serial.write(serialBuffer, pos);
        Serial.println();
#endif
        break;
      }
      if (_eventHandler) {
        _eventHandler(mode, serialBuffer, pos);
      }
      break;

      default:
      break;
    }
  }
}

void SerialPacketizer::onEvent(std::function<void (uint8_t mode, uint8_t *payload, size_t length)> eventHandler) {
  _eventHandler = eventHandler;
}

void SerialPacketizer::send(uint8_t mode, uint8_t *payload, size_t length) {
  serial->write(START_BYTE);
  serial->write(mode);
  serial->write(payload, length);
  serial->write(END_BYTE);
  uint8_t _checksum = 0;
  for (uint i=0; i<length; i++) {
    _checksum = _checksum ^ payload[i];
  }
  serial->write(_checksum);
}

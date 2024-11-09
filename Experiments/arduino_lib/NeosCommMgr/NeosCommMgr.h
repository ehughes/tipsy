#ifndef _NEOSCOMMMGR_H
#define _NEOSCOMMMGR_H

/*
  NeosCommMgr:

*/

#include <SerialPacketizer.h>
#include <ArduinoJson.h>
#include <functional>

#define MAX_PLAYERS 8

class NeosCommMgr {
public:
  struct PlayerDataCommon {
    uint32_t score;
    uint32_t color; // for now H as in HSV
  };
  struct GameDataCommon {
    const char *mode; // game type
    const char *state; // current state
    int8_t winner; // index of winning player
    uint8_t players_len;
    PlayerDataCommon players[MAX_PLAYERS];
  };
  NeosCommMgr(HardwareSerial *s);
  void process(void);
  void onConfigureGame(std::function<void (GameDataCommon *gameData)> configureGameHandler);
  void sendGameStatus(GameDataCommon *gameStatus);

protected:
  HardwareSerial *serial;
  SerialPacketizer *sp;
  char outputBuffer[BUFFER_SIZE];
  GameDataCommon _gameData;
  void packetRecieved(uint8_t mode, uint8_t *payload, size_t length);
  std::function<void (GameDataCommon *gameData)> _configureGameHandler;
};

#endif
#include "NeosCommMgr.h"

NeosCommMgr::NeosCommMgr(HardwareSerial *s) :
  serial(s)
{
  s->begin(9600);
  sp = new SerialPacketizer(serial);
  sp->onEvent(
    [this] (uint8_t mode, uint8_t *payload, size_t length) {
      packetRecieved(mode, payload, length);
    }
  );
}

void NeosCommMgr::process(void) {
  sp->process();
}

void NeosCommMgr::onConfigureGame(std::function<void (NeosCommMgr::GameDataCommon *gameData)> configureGameHandler) {
  _configureGameHandler = configureGameHandler;
}

void NeosCommMgr::packetRecieved(uint8_t mode, uint8_t *payload, size_t length) {
  Serial.printf("serial packet mode: %d length: %d", mode, length);
  Serial.println();
  Serial.write(payload, length);
  Serial.println();

  if (mode != 0) {
    return;
  }

  payload[length] = 0;
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char *)payload);

  // memset zero out data?

  // check type propery for "game_configure"

  if (root.containsKey("mode")) {
    _gameData.mode = root["mode"]; // copy?
    Serial.println(_gameData.mode);
  }

  if (root.containsKey("state")) {
    _gameData.state = root["state"]; // copy?
    Serial.println(_gameData.state);
  }

  JsonArray& nestedArray = root["players"];
  _gameData.players_len = nestedArray.size();
  Serial.printf("players_len: %d", _gameData.players_len);
  Serial.println();

  for (int i=0; i <_gameData.players_len; i++) {
    _gameData.players[i].score = nestedArray[i]["score"];
    _gameData.players[i].color = nestedArray[i]["color"];
    Serial.printf("player data: score %d color: %d", _gameData.players[i].score, _gameData.players[i].color);
    Serial.println();
  }

  if (_configureGameHandler) {
    _configureGameHandler(&_gameData);
  }
}

void NeosCommMgr::sendGameStatus(NeosCommMgr::GameDataCommon *gs) {
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["type"] = "game_status";

  root["winner"] = gs->winner;
  if (gs->state != NULL) {
    root["state"] = gs->state;
  }
  if (gs->mode != NULL) {
    root["mode"] = gs->mode;
  }

  JsonArray& playerarray = root.createNestedArray("players");
  for (int i=0; i<gs->players_len; i++) {
    JsonObject& playerObj = jsonBuffer.createObject();
    playerObj["score"] = gs->players[i].score;
    playerarray.add(playerObj);
  }

  root["time"] = millis();

  root.printTo(outputBuffer, sizeof(outputBuffer));
  sp->send(0, (uint8_t*)outputBuffer, strlen(outputBuffer));
}

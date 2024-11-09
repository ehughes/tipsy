#include "FastLED.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#include <ADC.h>
#include "RingBuffer.h"
#include <IntervalTimer.h>

#include <ArduinoJson.h>
#include <SerialPacketizer.h>
#include <NeosCommMgr.h>

//
// CONFIG DEFS
//
#define SP_MAX_PLAYERS 2

//#define TEST_RIG
#define ENABLE_AUDIO
#define ENABLE_WIFI

//
// GAME STUFF
//
struct PlayerData {
    float score;
    float hallValue;
    float lastHallValue;
    uint8_t pos;
    uint8_t lastPos;
    uint8_t color;
};
PlayerData players[SP_MAX_PLAYERS];
NeosCommMgr::PlayerDataCommon *pdc;

enum GameState {
    STARTUP,
    WAIT,
    INTRO,
    PLAY,
    END,
    FLAG
};
GameState currentState = STARTUP;

int8_t winner = -1;

elapsedMicros sincePhysicsDone;
elapsedMillis sinceLedUpdate;
elapsedMillis gameTimer;
elapsedMillis sinceGameStatus;

#define TIME_PERIOD 0.050 // time period between physics updates (seconds)
#define MAX_SCORE 100
#define ROT_SCORE 20.0
#define MIN_HALL 0.75
#define MAX_HALL 0.05

// LED STUFF
//
// How many leds in your strip? 240!!
#define NUM_LEDS_PER_PLAYER 300
#define NUM_LEDS (NUM_LEDS_PER_PLAYER*SP_MAX_PLAYERS)

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN_1 2
#define CLOCK_PIN_1 5
#define DATA_PIN_2 20
#define CLOCK_PIN_2 21
// Define the array of leds
CRGB leds[NUM_LEDS];

//
// AUDIO STUFF
//
#ifdef ENABLE_AUDIO
// audio stuff
// GUItool: begin automatically generated code
AudioPlaySdWav           playWav1;       //xy=154,78
AudioOutputI2S           i2s1;           //xy=334,89
AudioConnection          patchCord1(playWav1, 0, i2s1, 0);
AudioConnection          patchCord2(playWav1, 1, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=240,153
// GUItool: end automatically generated code
#define PLAY1(X) playWav1.play(X)
#else
#define PLAY1(X) Serial.println(X)
#endif

//
// HALL SENSOR STUFF
//
uint32_t saveTime; // micros for last sample
ADC *adc = new ADC(); // adc object
uint8_t hallPins[] = { A2, A3 };
uint8_t pinIdx = 0;
IntervalTimer adcTimer;
RingBuffer *hallBuffers[SP_MAX_PLAYERS];

//
// WIFI STUFF
//
#define WIFI_SERIAL Serial1
NeosCommMgr *ncm;
NeosCommMgr::GameDataCommon gdStatus;

void setup() {
  FastLED.addLeds<APA102, DATA_PIN_1, CLOCK_PIN_1, BGR, DATA_RATE_KHZ(4000)>(leds, 0, NUM_LEDS_PER_PLAYER);
  FastLED.addLeds<APA102, DATA_PIN_2, CLOCK_PIN_2, BGR, DATA_RATE_KHZ(500)>(leds, NUM_LEDS_PER_PLAYER, NUM_LEDS_PER_PLAYER);
  FastLED.show(); // Refresh strip

  Serial.begin(9600); //ignored
  delay(1000);
  Serial.println("HELLO");

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
#ifdef ENABLE_AUDIO
  AudioMemory(5);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.75);

  SPI.setMOSI(7);
  SPI.setSCK(14);
  if (!(SD.begin(10))) {
     Serial.println("Unable to access the SD card");
  }
#endif

  for (int i=0; i<SP_MAX_PLAYERS; i++) {
    pinMode(hallPins[i], INPUT);
    hallBuffers[i] = new RingBuffer;
  }

  adc->adc0->setReference(ADC_REF_3V3);
  adc->adc0->setAveraging(8); // set number of averages
  adc->adc0->setResolution(8); // set bits of resolution
  adc->adc0->setConversionSpeed(ADC_LOW_SPEED); // change the conversion speed
  adc->adc0->setSamplingSpeed(ADC_VERY_LOW_SPEED); // change the sampling speed

  adcTimer.begin(adctimer_callback, 10000); // 10ms
  adc->enableInterrupts(ADC_0);

  ncm = new NeosCommMgr(&WIFI_SERIAL);
  ncm->onConfigureGame(
    [] (NeosCommMgr::GameDataCommon *gameData) {
      Serial.println("onConfigureGame lambda");
      if (strncmp(gameData->mode, "StartGame", 9) == 0) {
        resetPlayerData();
        if (gameData->players_len>SP_MAX_PLAYERS) {
          Serial.println("too many players requested");
          gameData->players_len = SP_MAX_PLAYERS;
        }
        // set colors
        for (int i=0; i<gameData->players_len; i++) {
          players[i].color = gameData->players[i].color;
        }
        // go to intro
        currentState = INTRO;
        gameTimer = 0;
        Serial.println("INTRO");
      }
      if (strncmp(gameData->mode, "StopGame", 8) == 0) {
        resetPlayerData();
        // go to wait 
        currentState = WAIT;
        gameTimer = 0;
        Serial.println("WAIT");
      }
      if (strncmp(gameData->mode, "ShowFlag", 8) == 0) {
        players[0].color = gameData->players[0].color;
        // go to FLAG 
        currentState = FLAG;
        gameTimer = 0;
        Serial.println("FLAG");
      }
    }
  );

  sincePhysicsDone = 0;
  sinceLedUpdate = 0;
  gameTimer = 0;
  sinceGameStatus = 0;

  memset(players, 0, sizeof(PlayerData)*SP_MAX_PLAYERS);
  resetPlayerData();
}

void loop() {
  // process any incoming messages and send periodic status updates
  ncm->process();
  if (sinceGameStatus > 200) {
    memset(&gdStatus, 0, sizeof(NeosCommMgr::GameDataCommon));
    gdStatus.players_len = SP_MAX_PLAYERS;
    for (int i=0; i<SP_MAX_PLAYERS; i++) {
      gdStatus.players[i].score = players[i].score;
    }
    gdStatus.winner = winner;
    ncm->sendGameStatus(&gdStatus);
    sinceGameStatus = 0;
  }

  // check for any input from console
  if (Serial.available()) {
    uint8_t byte = Serial.read();
    if (byte == 'p') {
        int player = Serial.parseInt();
        int pled = Serial.parseInt();
        Serial.printf("P %d %d\r\n", player, pled);
        FastLED.clearData();
        setPlayerLed(player, pled, CRGB::White);
        FastLED.show();
        delay(5000);
        return;
    }
    for (int i=0; i<SP_MAX_PLAYERS; i++) {
      Serial.println(players[i].hallValue, DEC);
    }
    Serial.println(saveTime);
  }

  scanHalls();

  switch (currentState) {
    case STARTUP:
      if (gameTimer >= 2000) {
        for (int i=0; i<SP_MAX_PLAYERS; i++) {
          players[i].lastPos = players[i].pos;
        }
        currentState = INTRO;
        gameTimer = 0;
        Serial.println();
        Serial.println("INTRO");
    }
    break;

    // wait for configure message
    case WAIT:
    break;

    // game intro after configure message
    case INTRO:
    if (gameTimer >= 6000) {
        currentState = PLAY;
        gameTimer = 0;
        Serial.println("PLAY");
#ifdef ENABLE_AUDIO
        playWav1.play("BELL.WAV");
#endif
    }
    break;

    case PLAY:
    doPhysics();
    for (int i=0; i<SP_MAX_PLAYERS; i++) {
        if (players[i].score >= MAX_SCORE) {
            winner = i;
        }
    }
    if (winner > -1) {
        currentState = END;
        gameTimer = 0;
        Serial.println("END");
#ifdef ENABLE_AUDIO
        playWav1.play("SIREN.WAV");
#endif
    }
    break;

    case END:
    if (gameTimer >= 10000) {
        currentState = INTRO;
        gameTimer = 0;
        resetPlayerData();
        Serial.println("INTRO");
    }
    break;

    // wait for configure message
    case FLAG:
    break;
  }

  // update LEDS every 30ms
  if (sinceLedUpdate < 30) {
    return;
  }
  sinceLedUpdate = 0;
  FastLED.clearData();

  switch (currentState) {
    case STARTUP:
    break;

    case WAIT:
    renderWait();
    break;

    case INTRO:
    renderIntro();
    break;

    case PLAY:
    renderPlay();
    break;

    case END:
    renderEnd();
    break;

    case FLAG:
    renderFlag();
    break;
  }
  //FastLED.setBrightness(64);
  FastLED.show(); // Refresh strip
}

void resetPlayerData() {
    for (int i = 0; i < SP_MAX_PLAYERS; i++) {
        players[i].score = 0.0;
        players[i].hallValue = 0.0;
        players[i].lastHallValue = 0.0;
        players[i].color = 32 + i*128;
    }
    winner = -1;
}

// record value in player struct
void scanHalls() {
    for (int i = 0; i < SP_MAX_PLAYERS; i++) {
        while (!hallBuffers[i]->isEmpty()) {
            float value = (hallBuffers[i]->read()*1.0)/adc->getMaxValue(ADC_0);
            value = 1.0 - (value-MIN_HALL)/(MAX_HALL-MIN_HALL);
            if (value > 1.0) {
              value = 1.0;
            }
            if (value < 0.0) {
              value = 0.0;
            }
            players[i].pos = floor(7.0*value);
            players[i].hallValue = value;
        }
    }
}

// v = u + at
// s = ut + (1/2)at^2
void doPhysics() {
  if (sincePhysicsDone < 1000*1000*TIME_PERIOD) {
      return;
  }
  sincePhysicsDone = 0;
  for (int i = 0; i < SP_MAX_PLAYERS; i++) {
    if (players[i].pos != players[i].lastPos) {
      Serial.printf("%d %d %d\r\n", i, players[i].pos, players[i].lastPos);
      if (players[i].pos <2  && players[i].lastPos >5) {
        // clockwise score
        players[i].score += ROT_SCORE;
#ifdef ENABLE_AUDIO
        playWav1.play("BOING.WAV");
#endif
        Serial.println("CW");
      }
      else if (players[i].pos >5 && players[i].lastPos <2) {
        // counter clockwise score
        players[i].score += ROT_SCORE;
#ifdef ENABLE_AUDIO
        playWav1.play("BOING.WAV");
#endif
        Serial.println("CCW");
      }
      else if (players[i].pos > players[i].lastPos) {
        Serial.println("CW");
      }
      else {
        Serial.println("CCW");
      }
      players[i].lastPos = players[i].pos;
    }
  }
}


// 01234567
// SESESESE
// 00112233
void setPlayerLed(int player, uint8_t pled, CRGB color) {
  int stripStart = /*START_LED + */player*NUM_LEDS_PER_PLAYER;
  //int stripEnd = stripStart + NUM_LEDS_PER_PLAYER - 1;
  if (pled >= NUM_LEDS_PER_PLAYER) {
    return;
  }

  // index counting from bottom
  leds[stripStart + pled] = color;
}

void renderSpin(uint player, float speed, CRGB color) {
    // if (speed == 0.0) return;
    // uint ms = 100/abs(speed);
    // uint8_t pos = (gameTimer/ms)%18;
    uint8_t pos = players[player].hallValue * 17;
    if (player & 1) {
      pos = 17 - pos;
    }
    // if (speed < 0.0) {
    //   pos = 17 - pos;
    // }
    for (uint i = player*NUM_LEDS_PER_PLAYER; i < (player+1)*NUM_LEDS_PER_PLAYER; i++) {
      if (i % 18 == pos) {
        leds[i] = color;
      }
      if ((i+6) % 18 == pos) {
        leds[i] = color;
      }
      if ((i+12) % 18 == pos) {
        leds[i] = color;
      }
    }
}

void renderWait() {
  // renderSpin(0, 0.0, CRGB::White);
  // renderSpin(1, 0.0, CRGB::White);
}

void renderIntro() {
  uint8_t t = (gameTimer/5)%255;
  for (int i=0; i < 18; i++) {
    for (int j=0; j < 4; j++) {
      uint8_t v = sin8(t+j*63);
      setPlayerLed(0, j*18+i, CHSV(players[0].color, 255, v));
      setPlayerLed(1, j*18+i, CHSV(players[1].color, 255, v));
    }
  }
}

void renderPlay() {
  uint8_t t = (gameTimer/5)%255;

  uint8_t y = 4+lerp8by8(0, 12, 2.55*players[0].score);
  for (int i=0; i < 18; i++) {
    for (int j=0; j < y; j++) {
      uint8_t v = sin8(t+j*(255/y));
      setPlayerLed(0, j*18+i, CHSV(players[0].color, 255, v));
    }
  }

  y = 4+lerp8by8(0, 12, 2.55*players[1].score);
  for (int i=0; i < 18; i++) {
    for (int j=0; j < y; j++) {
      uint8_t v = sin8(t+j*(255/y));
      setPlayerLed(1, j*18+i, CHSV(players[1].color, 255, v));
    }
  }
}

void renderEnd() {
  uint8_t t = (gameTimer/10)%255;
  uint8_t y = lerp8by8(0, 240, sin8(t));

  for (int i=0; i < 9; i++) {
    setPlayerLed(winner, y+i, CRGB::White);
    setPlayerLed(winner, (239-y)+i, CRGB::White);
  }
}

void renderFlag() {
  uint8_t t = (gameTimer/10)%255;
  uint8_t y = lerp8by8(0, 240, sin8(t));
  CRGB flag_color = CHSV(players[0].color, 255, 255);

  for (int i=0; i < 9; i++) {
    setPlayerLed(0, y+i, flag_color);
    setPlayerLed(0, (239-y)+i, flag_color);
    setPlayerLed(1, y+i, flag_color);
    setPlayerLed(1, (239-y)+i, flag_color);
  }
}

// hold elapsed time for a single sample to be converted and buffered
elapsedMicros sampleTime;

void adctimer_callback(void) {

    sampleTime = 0;
    adc->startSingleRead(hallPins[pinIdx], ADC_0);

}

// when the measurement finishes, this will be called
// first: see which pin finished and then save the measurement into the correct buffer
void adc0_isr() {

    // uint8_t pin = ADC::sc1a2channelADC0[ADC0_SC1A&ADC_SC1A_CHANNELS]; // the bits 0-4 of ADC0_SC1A have the channel

    hallBuffers[pinIdx]->write(adc->readSingle());
    saveTime = sampleTime;
    // restore ADC config if it was in use before being interrupted by the analog timer
    // if (adc->adc0->adcWasInUse) {
    //     // restore ADC config, and restart conversion
    //     adc->adc0->loadConfig(&adc->adc0->adc_config);
    //     // avoid a conversion started by this isr to repeat itself
    //     adc->adc0->adcWasInUse = false;
    // }

    pinIdx++;
    if (pinIdx >= SP_MAX_PLAYERS) {
        pinIdx = 0;
    }
}

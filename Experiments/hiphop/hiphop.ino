#include <ADC.h>
#include <ADC_Module.h>
#include <RingBuffer.h>
#include <RingBufferDMA.h>

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
#define HH_MAX_PLAYERS 4
//#define TEST_RIG
#define ENABLE_AUDIO

//
// GAME STUFF
//
struct PlayerData {
    float score;
    float velocity;
    float threshold;
    float piezoValue;
    bool hitDetected;
    elapsedMicros hitTime;
    uint16_t hitCount;
    CRGB color;
};
PlayerData players[HH_MAX_PLAYERS];

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

elapsedMicros sincePiezoClear;
elapsedMicros sincePhysicsDone;
elapsedMillis sinceLedUpdate;
elapsedMillis sinceGameStatus;
elapsedMillis gameTimer;

#define JUMP_ACCEL 0.025 // acceleration of hit (pixels/seconds^2 upward)
#define ACCEL_PERIOD 0.250 // acceleration time for a single hit (seconds)
#define MAX_VELOCITY 0.0125
#define GRAVITY 0.012 // acceleration of gravity (pixels/seconds^2 downward)
#define TIME_PERIOD 0.020 // time period between physics updates (seconds)
#define SCAN_PERIOD 0.040 // time period for detecting a single hit (seconds)
#define MAX_SCORE 4.0

// LED STUFF
//
// How many leds in your strip?
#define NUM_LEDS (13*4+89*HH_MAX_PLAYERS)

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 2
#define CLOCK_PIN 5

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
#endif

//
// PIEZO STUFF
//
uint32_t saveTime; // micros for last sample
ADC *adc = new ADC(); // adc object
#define PIEZOPINS_LENGTH HH_MAX_PLAYERS
uint8_t piezoPins[] = { A6, A3, A2, A7 };
uint8_t pinIdx = 0;
IntervalTimer adcTimer;
RingBuffer *piezoBuffers[PIEZOPINS_LENGTH];

//
// WIFI STUFF
//
#define WIFI_SERIAL Serial1
NeosCommMgr *ncm;
NeosCommMgr::GameDataCommon gdStatus;

void setup() {
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR, DATA_RATE_KHZ(4000)>(leds, NUM_LEDS);
  FastLED.show(); // Refresh strip

  Serial.begin(9600);
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

  for (int i=0; i<PIEZOPINS_LENGTH; i++) {
    pinMode(piezoPins[i], INPUT);
    piezoBuffers[i] = new RingBuffer;
  }

  adc->adc0->setReference(ADC_REF_1V2);
  adc->adc0->setAveraging(8); // set number of averages
  adc->adc0->setResolution(8); // set bits of resolution
  adc->adc0->setConversionSpeed(ADC_LOW_SPEED); // change the conversion speed
  // low sampling speed for high impedance source
  adc->adc0->setSamplingSpeed(ADC_VERY_LOW_SPEED); // change the sampling speed

  adcTimer.begin(adctimer_callback, 250); // 250 us
  adc->enableInterrupts(ADC_0);

  sincePiezoClear = 0;
  sincePhysicsDone = 0;
  sinceLedUpdate = 0;
  sinceGameStatus = 0;
  gameTimer = 0;

  memset(players, 0, sizeof(PlayerData)*HH_MAX_PLAYERS);
  resetPlayerData();
  
  ncm = new NeosCommMgr(&WIFI_SERIAL);
  ncm->onConfigureGame(
    [] (NeosCommMgr::GameDataCommon *gameData) {
      Serial.println("onConfigureGame lambda");
      if (strncmp(gameData->mode, "StartGame", 9) == 0) {
        resetPlayerData();
        if (gameData->players_len>HH_MAX_PLAYERS) {
          Serial.println("too many players requested");
          gameData->players_len = HH_MAX_PLAYERS;
        }
        // set colors
        for (int i=0; i<gameData->players_len; i++) {
          players[i].color = CHSV(gameData->players[i].color, 255, 255);
          Serial.printf("player %d color %x %x %x\r\n", i, players[i].color.r, players[i].color.g, players[i].color.b);
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
        players[0].color = CHSV(gameData->players[0].color, 255, 255);
        // go to FLAG 
        currentState = FLAG;
        gameTimer = 0;
        Serial.println("FLAG");
      }
    }
  );
}

void loop() {
  // process any incoming messages and send periodic status updates
  ncm->process();
  if (sinceGameStatus > 200) {
    memset(&gdStatus, 0, sizeof(NeosCommMgr::GameDataCommon));
    gdStatus.players_len = HH_MAX_PLAYERS;
    for (int i=0; i<HH_MAX_PLAYERS; i++) {
      gdStatus.players[i].score = 25*players[i].score;
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
    for (int i=0; i<PIEZOPINS_LENGTH; i++) {
      Serial.println(players[i].piezoValue, DEC);
    }
    Serial.println(saveTime);
  }

  scanPiezos();

  switch (currentState) {
    // initialize and calibrate
    case STARTUP:
    sincePiezoClear = 0; // make sure max values not reset for now
    if (gameTimer >= 2000) {
        Serial.println("THRESHOLDS:");
        for (int i = 0; i < HH_MAX_PLAYERS; i++) {
            players[i].threshold = 3.5*players[i].piezoValue;
            Serial.printf("%f\t", players[i].threshold);
        }
        currentState = INTRO;
        gameTimer = 0;
        Serial.println();
        Serial.println("INTRO");
        return;
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

    // actual game play
    case PLAY:
    scanHits();
    doPhysics();
    for (int i=0; i<HH_MAX_PLAYERS; i++) {
        if (players[i].score > MAX_SCORE) {
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

    // game over, go back to wait state
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
    for (int i = 0; i < HH_MAX_PLAYERS; i++) {
        players[i].score = 0.0;
        players[i].velocity = 0.0;
        players[i].piezoValue = 0.0;
        players[i].hitDetected = false;
        players[i].hitCount = 0;
        players[i].color = CHSV(i*64, 255, 255);
    }
    winner = -1;
}

// record peak value in player struct
void scanPiezos() {
    for (int i = 0; i < HH_MAX_PLAYERS; i++) {
        while (!piezoBuffers[i]->isEmpty()) {
            float value = (piezoBuffers[i]->read()*1.2)/adc->getMaxValue(ADC_0);
            if (value > players[i].piezoValue) {
                players[i].piezoValue = value;
            }
        }
        if (sincePiezoClear >= 1000*1000*SCAN_PERIOD) {
            players[i].piezoValue = 0.0;
            sincePiezoClear = 0;
            //Serial.print("*");
        }
    }
}

bool scanHits() {
    bool found = false;
    for (int i = 0; i < HH_MAX_PLAYERS; i++) {
        if (players[i].piezoValue >= players[i].threshold &&
            !players[i].hitDetected) {
#ifdef ENABLE_AUDIO
            playWav1.play("BOING.WAV");
#endif
            Serial.println("HIT ON");
            Serial.println(i, DEC);
            Serial.println(players[i].piezoValue, DEC);
            players[i].hitDetected = true;
            players[i].hitTime = 0;
            players[i].hitCount++;
            found = true;
        }
    }
    return found;
}

// v = u + at
// s = ut + (1/2)at^2
void doPhysics() {
    if (sincePhysicsDone < 1000*1000*TIME_PERIOD) {
        return;
    }
    sincePhysicsDone = 0;
    for (int i = 0; i < HH_MAX_PLAYERS; i++) {
        float t = TIME_PERIOD;
        float a = 0.0 - GRAVITY;
        if (players[i].hitDetected) {
            a += JUMP_ACCEL;
            if (players[i].hitTime >= 1000*1000*ACCEL_PERIOD) {
                Serial.println("HIT OFF");
                players[i].hitDetected = false;
            }
        }
        players[i].score = players[i].score + players[i].velocity + 0.5*a*t*t;
        players[i].velocity = players[i].velocity + a*t;
        if (players[i].velocity > MAX_VELOCITY) {
            //Serial.println("MAX");
            players[i].velocity = MAX_VELOCITY;
        }
        if (players[i].velocity < 0.0 - 1.5*MAX_VELOCITY) {
            //Serial.println("MAX");
            players[i].velocity = 0.0 - 1.5*MAX_VELOCITY;
        }
        if (players[i].score <= 0.0) {
            players[i].score = 0.0;
            players[i].velocity = 0.0;
        }
    }
}

#ifdef TEST_RIG
#define START_LED 0
#undef UPDOWN
#else
#define UPDOWN
#define START_LED 13*4
#endif
#define LEDS_PER_PLAYER 89

// 01234567
// SESESESE
// 00112233
void setPlayerLed(int player, uint8_t pled, CRGB color) {
  int stripStart = START_LED + player*LEDS_PER_PLAYER;
  int stripEnd = stripStart + LEDS_PER_PLAYER - 1;
  if (pled >= LEDS_PER_PLAYER) {
    return;
  }
#ifdef UPDOWN
  if ((player & 1) == 1) {
#endif
    // index counting from bottom
    leds[stripStart + pled] = color;
#ifdef UPDOWN
  } else {
    // index counting from top
    leds[stripEnd - pled] = color;
  }
#endif
}

void renderWait() {
    // for (int i=0; i<HH_MAX_PLAYERS; i++) {
    //     setPlayerDisplay(i, players[i].piezoValue*2.0);
    // }
}

void renderIntro() {

    // red yellow green "countdown"
    CRGB color;
    switch (gameTimer/2000) {
        case 0:
        color = CRGB::Red;
        break;

        case 1:
        color = CRGB::Yellow;
        break;

        case 2:
        default:
        color = CRGB::Green;
        break;
    }
    uint8_t pos = ((gameTimer%1000)/72)%13;
    pos = pos + 6;
    if (pos > 12) {
        pos = pos - 13;
    }
    if (pos >= 7) {
        pos = 12 - pos;
    }
    for (int i = 0; i < HH_MAX_PLAYERS; i++) {
        leds[i*13+pos] = color;
        leds[i*13+(12-pos)] = color;
    }

    // render player colors
    uint8_t mod = (gameTimer/20)%8;
    for (int i=0; i<HH_MAX_PLAYERS; i++) {
      // Serial.printf("player %d color %x %x %x\r\n", i, players[i].color.r, players[i].color.g, players[i].color.b);
      for (int j=0; j<LEDS_PER_PLAYER; j++) {
          if ((j+mod)%8 == 0) {
              setPlayerLed(i, j, players[i].color);
          }
      }
    }
}

void setPlayerDisplay(int player, float value) {
  value = value/MAX_SCORE;
  //uint8_t hue = value * 255;
  uint8_t pled = value * LEDS_PER_PLAYER;
#ifdef TEST_RIG
  if (pled >= 9) {
    pled = 9;
  }
#endif
  for (int j=0; j<3; j++) {
    int i = random(6);
    //hue = hue + 2*i;
    setPlayerLed(player, pled + i, players[player].color);
  }
}

void renderPlay() {
    for (int i=0; i<HH_MAX_PLAYERS; i++) {
        setPlayerDisplay(i, players[i].score);
    }
}

void renderEnd() {
    uint8_t tail = (gameTimer/20)%(13*4);
    uint8_t head = tail + 10;
    for (int i=tail; i<head; i++) {
        uint x = i;
        if (x > (13*4-1)) {
            x = x - 13*4;
        }
        leds[x] = CRGB::White;
    }

    uint8_t mod = (gameTimer/20)%8;
    for (int j=0; j<LEDS_PER_PLAYER; j++) {
        if ((j+mod)%8 == 0) {
            setPlayerLed(winner, j, CRGB::White);
        }
    }
}

void renderFlag() {
    uint8_t tail = (gameTimer/20)%(13*4);
    uint8_t head = tail + 10;
    for (int i=tail; i<head; i++) {
        uint x = i;
        if (x > (13*4-1)) {
            x = x - 13*4;
        }
        leds[x] = players[0].color;
    }

    uint8_t mod = (gameTimer/20)%8;
    for (int i=0; i<HH_MAX_PLAYERS; i++) {
      for (int j=0; j<LEDS_PER_PLAYER; j++) {
          if ((j+mod)%8 == 0) {
              setPlayerLed(i, j, players[0].color);
          }
      }
    }
}

// hold elapsed time for a single sample to be converted and buffered
elapsedMicros sampleTime;

void adctimer_callback(void) {

    sampleTime = 0;
    adc->startSingleRead(piezoPins[pinIdx], ADC_0);

}

// when the measurement finishes, this will be called
// first: see which pin finished and then save the measurement into the correct buffer
void adc0_isr() {

    // uint8_t pin = ADC::sc1a2channelADC0[ADC0_SC1A&ADC_SC1A_CHANNELS]; // the bits 0-4 of ADC0_SC1A have the channel

    piezoBuffers[pinIdx]->write(adc->readSingle());
    saveTime = sampleTime;
    // restore ADC config if it was in use before being interrupted by the analog timer
    // if (adc->adc0->adcWasInUse) {
    //     // restore ADC config, and restart conversion
    //     adc->adc0->loadConfig(&adc->adc0->adc_config);
    //     // avoid a conversion started by this isr to repeat itself
    //     adc->adc0->adcWasInUse = false;
    // }

    pinIdx++;
    if (pinIdx >= PIEZOPINS_LENGTH) {
        pinIdx = 0;
    }
}

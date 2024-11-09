#include "TipsyConfig.h"
#include <IntervalTimer.h>
#include <SerialFlash.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <ADC.h>
#include "RingBuffer.h"
#include "eGFX.h"
#include "eGFX_Driver_APA102_28_28.h"
#include <ArduinoJson.h>
#include <SerialPacketizer.h>
#include <NeosCommMgr.h>
#include "tipsy_32BPP.h"
#include "UserInput.h"
#include "Game_SensorTest.h"
#include "FONT_3_5_1BPP.h"
#include "Game_BubblePop.h"
#include "ScoreDisplay.h"
#include "Selector.h"
#include "GameSupport.h"
#include "StarCatcher.h"

#define MAX_NUM_PLAYERS 1

#define LED_FRAME_RATE                   4
#define SWITCH_PIN                      3

#define TIPSY_RESET                      0
#define TIPSY_STATE_INIT                 1
#define TIPSY_STATE_PLAY                 2
#define TIPSY_STATE_VICTORY              3
#define TIPSY_STATE_DISPLAY_FLAG         4

#define ACCEL_Y_CHANNEL   A6
#define ACCEL_X_CHANNEL   A3
#define POT_CHANNEL       A2

#define ONE_SECOND                  100

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#define NUM_SYSTEM_TIMERS 	8

#define GAME_INPUT_TEST     0
#define GAME_BUBBLE_POP     1
#define GAME_BUBBLE_MAZE    2
#define GAME_STAR_CATCHER   3
#define GAME_SCORE_DISPLAY  4
#define GAME_SELECTOR       5


extern char StringBuf[256];

extern uint8_t SelectedGame;
extern volatile int32_t SystemTimers[NUM_SYSTEM_TIMERS];
void ChangeTipsyState(uint8_t NewState);


 extern AudioPlaySdWav playWav1;
 extern AudioPlaySdWav playWav2;



void InitSystem();

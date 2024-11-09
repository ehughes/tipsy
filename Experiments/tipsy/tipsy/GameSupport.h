#include <stdint.h>

#ifndef __GAME_SUPPORT
#define __GAME_SUPPORT

extern uint8_t GlowIdx;

extern uint8_t GlowTable[256];

#define PLAYER_HISTORY_SIZE 16
#define VELOCITY_DIVIDER   18 
#define _CONTROL_1

typedef struct {

float x;
float y;

float vx;
float vy;

float X_History[PLAYER_HISTORY_SIZE];
float Y_History[PLAYER_HISTORY_SIZE];

uint32_t HistoryIndex;
uint32_t Color;
uint32_t Score;

}PlayerInfo;

extern PlayerInfo Player;

void ProcessStandardInput();




#endif

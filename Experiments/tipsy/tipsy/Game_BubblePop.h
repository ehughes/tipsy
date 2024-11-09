#ifndef _GAME_BUBBLE_POP
#define _GAME_BUBBLE_POP


void Process__Game__BubblePop();
void Init__Game__BubblePop();


typedef struct 
{
    float x;
    float y;
    
    int32_t vx;
    int32_t vy;    
        
    float  TargetSize;
    float  CurrentSize;
    
    int32_t State;
    int32_t TTL;
    
    uint32_t Color;
}Bubble;

#endif
#include "System.h"

#define SCORE_TIMER            SystemTimers[1]
#define SCORE_REFRESH_TIMER    SystemTimers[2]
#define SCORE_SHOW_TIME        ONE_SECOND*5


uint32_t MarqueePosition = 0;

uint32_t ScoreLevelAchieved;
uint32_t ScoreAchieved;


char * GetAchievementString(uint32_t Level)
{
    char * S;
    
    switch(Level)
    {
        default:
        S = "???";
        break;
        
        case GREEN_LEVEL:
            S = "Green";
         break;
        
        case YELLOW_LEVEL:
          S = "YELLOW";
         break;
        
        case  RED_LEVEL:
           S = "RED";
         break;
       
        case BLUE_LEVEL:
            S = "BLUE";
         break;
   
        case PURPLE_LEVEL:
      
        S = "PURPLE";
   
         break;
   
   
        case  WHITE_LEVEL :
         
         S = "WHITE";
   
         break;
   
    }
    
    return S;
}

void ShowScoreDisplay(uint32_t Level, uint32_t Score, char *Msg)
{
     SCORE_TIMER = 0;

     SelectedGame = GAME_SCORE_DISPLAY;
    
    ScoreAchieved = Score;
    
    ScoreLevelAchieved = Level;
    
    ChangeTipsyState(TIPSY_STATE_INIT);
   
    playWav1.play("END_GAME.wav");
   
}


void ProcessScoreDisplay()
{



    if(SCORE_REFRESH_TIMER > 0)
    {
    
      eGFX_ImagePlane_Clear(&eGFX_BackBuffer);
         SCORE_REFRESH_TIMER = 0;
        
         //Draw the marqee
            eGFX_PutPixel(&eGFX_BackBuffer,MarqueePosition,0,ScoreLevelAchieved);
         
           eGFX_PutPixel(&eGFX_BackBuffer,eGFX_PHYSICAL_SCREEN_SIZE_X - 1 - MarqueePosition,
                         eGFX_PHYSICAL_SCREEN_SIZE_Y-1,ScoreLevelAchieved);
           eGFX_PutPixel(&eGFX_BackBuffer,0,MarqueePosition,ScoreLevelAchieved);
           eGFX_PutPixel(&eGFX_BackBuffer,eGFX_PHYSICAL_SCREEN_SIZE_X-1,
                         eGFX_PHYSICAL_SCREEN_SIZE_Y- 1 - MarqueePosition, ScoreLevelAchieved);
           
           MarqueePosition++;
           if(MarqueePosition>=eGFX_PHYSICAL_SCREEN_SIZE_Y)
                MarqueePosition = 0;
    
    
        eGFX_printf_HorizontalCentered_Colored(&eGFX_BackBuffer,
                                               3,
                                               &FONT_3_5_1BPP,
                                               ScoreLevelAchieved,
                                               "%s",GetAchievementString(ScoreLevelAchieved)); 

        eGFX_printf_HorizontalCentered_Colored(&eGFX_BackBuffer,
                                               14,
                                               &FONT_3_5_1BPP,
                                               eGFX_PIXEL_RANDOM,
                                               "%i",ScoreAchieved); 
                                               
                                                  eGFX_Dump(&eGFX_BackBuffer);
                                       
    
    } 

   if(SCORE_TIMER > SCORE_SHOW_TIME)
   {
        SelectedGame = GAME_SELECTOR;
        ChangeTipsyState(TIPSY_STATE_INIT);
   }

   
     

}


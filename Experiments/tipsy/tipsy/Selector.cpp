#include "System.h"

#define SELECTOR_REFRESH_TIMER    SystemTimers[2]
#define SELECTOR_GLOW_TIMER    SystemTimers[0]


void Init__GameSelector()
{
    Player.x = 14;
}

void Process__Selector()
{
   
    if(SELECTOR_GLOW_TIMER > 0)
    {
        SELECTOR_GLOW_TIMER = 0;
        GlowIdx+=2;
    }
       
    
   
    if(SELECTOR_REFRESH_TIMER > 1)
    {
    
        
        SELECTOR_REFRESH_TIMER = 0;
        UpdateUserInput();
        ProcessStandardInput();
   
   
        eGFX_ImagePlane_Clear(&eGFX_BackBuffer);

        
          eGFX_Blit(&eGFX_BackBuffer,
            0,
            0,
            &Sprite_32BPP_TipsyHomeScreen);
            
           
        if(Player.x > 24)
            Player.x = 24;
            
        if(Player.x < 3)
            Player.x = 3;
          
    
          
      
        
         for(int i=0;i<PLAYER_HISTORY_SIZE;i++)
        {
           
            eGFX_PutPixel(&eGFX_BackBuffer,Player.X_History[i],19,GlowTable[GlowIdx] | GlowTable[GlowIdx]<<8 | GlowTable[GlowIdx]<<16);
         }
          eGFX_PutPixel(&eGFX_BackBuffer,Player.x,19,(uint32_t)GlowTable[GlowIdx] | (uint32_t)GlowTable[GlowIdx]<<8 | (uint32_t)GlowTable[GlowIdx]<<16);
        
        if((int)Player.x >=3 && (int)Player.x<=5)
        {
            SelectedGame = GAME_BUBBLE_POP;
            ChangeTipsyState(TIPSY_STATE_INIT);
        }
        
            if((int)Player.x >=22 && (int)Player.x<=25)
        {
            SelectedGame = GAME_STAR_CATCHER;
            ChangeTipsyState(TIPSY_STATE_INIT);
        }
        
        eGFX_Dump(&eGFX_BackBuffer);
    } 

 
   
     

}


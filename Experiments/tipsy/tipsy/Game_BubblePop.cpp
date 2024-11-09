#include "System.h"


#define BUBBLE_STATE_ENTERING   1
#define BUBBLE_STATE_ACTIVE     2
#define BUBBLE_STATE_EXITING    3
#define BUBBLE_STATE_INACTIVE   0


#define BUBBLE_GAME_TIMER           SystemTimers[1]
#define BUBBLE_LEVELUP_TIMER        SystemTimers[2]
#define BUBBLE_RESPAWN_TIMER        SystemTimers[3]
#define BUBBLE_REFRESH_TIMER       SystemTimers[4]
#define BUBBLE_SCORE_SHOW_TIMER     SystemTimers[5]

#define BUBBLE_GAME_LENGTH            (60*ONE_SECOND)
#define BUBBLE_GAME_LEVELUP_TIME      (5*ONE_SECOND)

#define LEVEL_UP_MARQUEE_TIME 200

#define  SCORE_SHOW_TIME 100
#define BUBBLE_RESPAWN_TIME    (100)

#define MAX_BUBBLES 4



#define GREEN_LEVEL_THRESHOLD       0
#define YELLOW_LEVEL_THRESHOLD      3
#define RED_LEVEL_THRESHOLD         6
#define BLUE_LEVEL_THRESHOLD        9
#define PURPLE_LEVEL_THRESHOLD      12
#define WHITE_LEVEL_THRESHOLD       15



uint32_t CurrentBubbleLevel =  GREEN_LEVEL;
   

#define BUBBLE_IN_SOUND "whoop.wav"
#define BUBBLE_OUT_SOUND "whoop_r.wav"
#define BUBBLE_POP_SOUND "Score.wav"
#define LEVEL_UP_SOUND  "LevelUp.wav"


  
    

uint8_t GameNumBubbles = 1;

Bubble MyBubbles[MAX_BUBBLES];

void ResetBubbles()
{
    int32_t i;
    
    for(i=0;i<MAX_BUBBLES;i++);
    {
      //  MyBubbles[i].x = 0;
       // MyBubbles[i].y = 0;
        //MyBubbles[i].TargetSize = 0;
        //MyBubbles[i].CurrentSize = 0;
        
        MyBubbles[i].State = BUBBLE_STATE_INACTIVE;
       // MyBubbles[i].TTL = 0;
    }
    
    Serial.println("Resetting Bubbles");

    
}

int32_t GetActiveBubbles()
{
    int32_t i,Count;
    
    Count = 0;
    
    for(i=0;i<MAX_BUBBLES;i++)
    {
        if(MyBubbles[i].State != BUBBLE_STATE_INACTIVE)
        {
            Count++;
        }
    }

    return Count;
    
}

int32_t GetNextBubbleSlot()
{
    int32_t i;
    
    for(i=0;i<MAX_BUBBLES;i++)
    {
        if(MyBubbles[i].State == BUBBLE_STATE_INACTIVE)
        {
            return i;
        }
    }

    return 0;
}

void SpawnNewBubble()
{
    int32_t i;
    
    i = GetNextBubbleSlot();
    
    MyBubbles[i].x = (rand() % eGFX_PHYSICAL_SCREEN_SIZE_X);
    MyBubbles[i].y = (rand() % eGFX_PHYSICAL_SCREEN_SIZE_Y);
    
    
  
    
    MyBubbles[i].CurrentSize = 0;
    
    MyBubbles[i].State = BUBBLE_STATE_ENTERING;
    MyBubbles[i].TTL = rand()%200 + 200;
    

    switch(CurrentBubbleLevel)
    {
        default:
        case GREEN_LEVEL:
         MyBubbles[i].Color = GREEN_LEVEL;
              MyBubbles[i].TargetSize = 6;
   
         break;
        
        case YELLOW_LEVEL:
        MyBubbles[i].Color = YELLOW_LEVEL;
             MyBubbles[i].TargetSize = 5;
   
         break;
        
        case  RED_LEVEL:
         MyBubbles[i].Color = RED_LEVEL;
              MyBubbles[i].TargetSize = 4;
   
         break;
       
        case BLUE_LEVEL:
           MyBubbles[i].Color = BLUE_LEVEL;
                MyBubbles[i].TargetSize = 3;
   
         break;
   
        case PURPLE_LEVEL:
           MyBubbles[i].Color = PURPLE_LEVEL;
                MyBubbles[i].TargetSize = 2;
   
         break;
   
   
        case  WHITE_LEVEL :
         MyBubbles[i].Color = WHITE_LEVEL;
              MyBubbles[i].TargetSize = 1;
   
         break;
   
    }
    
   playWav1.play(BUBBLE_IN_SOUND);

}

void BubbleSpawnProcess()
{
    if(    (GetActiveBubbles() < GameNumBubbles)
        && ( BUBBLE_RESPAWN_TIMER > BUBBLE_RESPAWN_TIME))
    {
       SpawnNewBubble(); 
       
       BUBBLE_RESPAWN_TIMER = 0;
    }
}

void BubbleActivityProcess()
{
  int32_t i;
  
  for(i = 0; i<MAX_BUBBLES;i++)
  {
    switch(MyBubbles[i].State)
    {
         case BUBBLE_STATE_ENTERING   :
         if(MyBubbles[i].CurrentSize<MyBubbles[i].TargetSize)
         {
            MyBubbles[i].CurrentSize += 0.2;
         }
         else
         {
         MyBubbles[i].State = BUBBLE_STATE_ACTIVE;
         }
         break;
         
         case BUBBLE_STATE_ACTIVE     :
         if(MyBubbles[i].TTL > 0)
         {
            MyBubbles[i].TTL-- ;
         }
         else
         {
          playWav1.play(BUBBLE_OUT_SOUND);
           MyBubbles[i].State =  BUBBLE_STATE_EXITING;
         }
         
         break;
         
         case BUBBLE_STATE_EXITING    :
         
          if(MyBubbles[i].CurrentSize> 0)
          {
                MyBubbles[i].CurrentSize -= 0.2;
          }
          else
          {
                BUBBLE_RESPAWN_TIMER = 0;
                MyBubbles[i].State = BUBBLE_STATE_INACTIVE;
          }
         break;
         
         default:
         case BUBBLE_STATE_INACTIVE   :
        break;
    }
  
  }
}

void KillBubble(uint32_t i)
{
    
    MyBubbles[i].State = BUBBLE_STATE_EXITING;
    
};

void RenderBubbles()
{

int i;

for(i = 0; i<MAX_BUBBLES;i++)
  {
  
   if((MyBubbles[i].CurrentSize>0) && (MyBubbles[i].State != BUBBLE_STATE_INACTIVE))
   {
   
   eGFX_Box B;
   
   B.P1.X = MyBubbles[i].x - MyBubbles[i].CurrentSize;
   B.P2.X = MyBubbles[i].x + MyBubbles[i].CurrentSize;
   B.P1.Y = MyBubbles[i].y - MyBubbles[i].CurrentSize;
   B.P2.Y = MyBubbles[i].y + MyBubbles[i].CurrentSize;
      
    eGFX_DrawBox(&eGFX_BackBuffer,
                        &B,
                        MyBubbles[i].Color);
                        
   }
  }
}
void Init__Game__BubblePop()
{
	ResetBubbles();

    GameNumBubbles = 1;
          
    BUBBLE_GAME_TIMER = 0;
    BUBBLE_RESPAWN_TIMER = 0;
    BUBBLE_LEVELUP_TIMER = 0;
    BUBBLE_REFRESH_TIMER = 0;

    Player. Color = 0x8000FF;
    Player.x = 14;
    Player.y = 14;
    Player.Score = 0;
    CurrentBubbleLevel = GREEN_LEVEL;
    BUBBLE_SCORE_SHOW_TIMER = SCORE_SHOW_TIME+1;
    BUBBLE_LEVELUP_TIMER = LEVEL_UP_MARQUEE_TIME;
}


void CheckBubbleCollision()
{
    uint32_t i;
    
    for(i=0;i<MAX_BUBBLES;i++)
    {
        if((Player.x >= MyBubbles[i].x - MyBubbles[i].TargetSize) &&
            (Player.x <= MyBubbles[i].x + MyBubbles[i].TargetSize) &&
            (Player.y >= MyBubbles[i].y - MyBubbles[i].TargetSize) &&
            (Player.y <= MyBubbles[i].y + MyBubbles[i].TargetSize) &&
            (MyBubbles[i].State == BUBBLE_STATE_ACTIVE))
            {
                KillBubble(i);
                BUBBLE_SCORE_SHOW_TIMER = 0;
    
             Player.Score++;
         
             if(Player.Score== YELLOW_LEVEL_THRESHOLD)
                {
                  playWav1.play(LEVEL_UP_SOUND);
                  BUBBLE_LEVELUP_TIMER =0;
                  CurrentBubbleLevel = YELLOW_LEVEL;
                }
                else if (Player.Score== RED_LEVEL_THRESHOLD )
                {
                     CurrentBubbleLevel = RED_LEVEL;
                      playWav1.play(LEVEL_UP_SOUND);
                     BUBBLE_LEVELUP_TIMER = 0;
                }
                else if (Player.Score== BLUE_LEVEL_THRESHOLD)
                {
                      CurrentBubbleLevel = BLUE_LEVEL;
                       playWav1.play(LEVEL_UP_SOUND);
                       
                       BUBBLE_LEVELUP_TIMER =0;
                }
                else if (Player.Score == PURPLE_LEVEL_THRESHOLD)
                {
                 CurrentBubbleLevel = PURPLE_LEVEL;
                  playWav1.play(LEVEL_UP_SOUND);
                     BUBBLE_LEVELUP_TIMER =0;
                }
                else if (Player.Score == WHITE_LEVEL_THRESHOLD)
                {
                 CurrentBubbleLevel = WHITE_LEVEL;
                  playWav1.play(LEVEL_UP_SOUND);
                  BUBBLE_LEVELUP_TIMER =0;
                }
                else
                {
                 playWav1.play(BUBBLE_POP_SOUND);
                 }
           
            }
      }

}


void Process__Game__BubblePop()
{

   
    if(BUBBLE_REFRESH_TIMER > 0)
    {
        UpdateUserInput();
   
        BUBBLE_REFRESH_TIMER = 0;
        
        ProcessStandardInput();

        eGFX_ImagePlane_Clear(&eGFX_BackBuffer);
      
        
        BubbleSpawnProcess();
       
        RenderBubbles();
        
        CheckBubbleCollision();
       
        for(int i=0;i<PLAYER_HISTORY_SIZE;i++)
        {
           
            eGFX_PutPixel(&eGFX_BackBuffer,Player.X_History[i],Player.Y_History[i],~Player.Color);
         }
       
        eGFX_PutPixel(&eGFX_BackBuffer,Player.x,Player.y+1,CurrentBubbleLevel);
        eGFX_PutPixel(&eGFX_BackBuffer,Player.x,Player.y-1,CurrentBubbleLevel);
        eGFX_PutPixel(&eGFX_BackBuffer,Player.x+1,Player.y,CurrentBubbleLevel);
        eGFX_PutPixel(&eGFX_BackBuffer,Player.x-1,Player.y,CurrentBubbleLevel);
        eGFX_PutPixel(&eGFX_BackBuffer,Player.x,Player.y,CurrentBubbleLevel);
       
       
       if( BUBBLE_LEVELUP_TIMER < LEVEL_UP_MARQUEE_TIME)
       {
           eGFX_PutPixel(&eGFX_BackBuffer,MarqueePosition,0,CurrentBubbleLevel);
         
          eGFX_PutPixel(&eGFX_BackBuffer,eGFX_PHYSICAL_SCREEN_SIZE_X - 1 - MarqueePosition,
                         eGFX_PHYSICAL_SCREEN_SIZE_Y-1,CurrentBubbleLevel);
           eGFX_PutPixel(&eGFX_BackBuffer,0,MarqueePosition,CurrentBubbleLevel);
           eGFX_PutPixel(&eGFX_BackBuffer,eGFX_PHYSICAL_SCREEN_SIZE_X-1,
                         eGFX_PHYSICAL_SCREEN_SIZE_Y- 1 - MarqueePosition, CurrentBubbleLevel);
           
           MarqueePosition++;
           if(MarqueePosition>=eGFX_PHYSICAL_SCREEN_SIZE_Y)
            MarqueePosition = 0;
           
       }
        BubbleActivityProcess();
    
        /*
        if(BUBBLE_SCORE_SHOW_TIMER < SCORE_SHOW_TIME)
        {
           eGFX_printf_Colored(&eGFX_BackBuffer,
                     Player.x,
                     Player.y,
                     &FONT_3_5_1BPP,
                     rand()&0xffff,
                     "+1");     
        }*/
      
        if(BUBBLE_GAME_TIMER > BUBBLE_GAME_LENGTH)
        {

            ShowScoreDisplay(CurrentBubbleLevel, Player.Score,"");
        }
            
     
        eGFX_Dump(&eGFX_BackBuffer);
        
    }
  
    
  //  if(BUBBLE_LEVELUP_TIMER > BUBBLE_GAME_LEVELUP_TIME)
    //{
      //  BUBBLE_LEVELUP_TIMER = 0;
       
       // if(GameNumBubbles<MAX_BUBBLES)
         //   GameNumBubbles++;
   //}
    
    //if(BUBBLE_GAME_TIMER > BUBBLE_GAME_LENGTH)
    //{

       // Init__Game__BubblePop();
   // }
}
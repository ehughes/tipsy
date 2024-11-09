#include "System.h"


char StarCatcherGameTimeString[4];

#define STAR_CATCHER_GAME_LOGIC_TIMER			 SystemTimers[0]
#define STAR_CATCHER_GAME_TIMER					 SystemTimers[1]
#define STAR_CATCHER_RED_SCORE_DISPLAY_TIMER	 SystemTimers[2]
#define STAR_CATCHER_GREEN_SCORE_DISPLAY_TIMER	 SystemTimers[3]
#define STAR_CATCHER_SOUND_LOCKOUT_TIMER         SystemTimers[4]
#define STARCATCHER_LEVELUP_TIMER                SystemTimers[5] 
#define GREEN_LEVEL_THRESHOLD       0
#define YELLOW_LEVEL_THRESHOLD      4
#define RED_LEVEL_THRESHOLD         8
#define BLUE_LEVEL_THRESHOLD        12
#define PURPLE_LEVEL_THRESHOLD      16
#define WHITE_LEVEL_THRESHOLD       20


#define LEVEL_UP_MARQUEE_TIME   200
#define STAR_CATCHER_SCORE_DISPLAY_TIME		     100
#define STAR_CATCHER_MAX_TIME					 60

#define BOUNCES_BEFORE_STAR_DESTRUCTION			 3

#define BOUNCE_0_SOUND   "BOUNCE_0.wav"
#define BOUNCE_1_SOUND   "BOUNCE_1.wav"
#define BOUNCE_2_SOUND   "BOUNCE_2.wav"
#define BOUNCE_3_SOUND   "BOUNCE_3.wav"
#define BOUNCE_4_SOUND   "BOUNCE_4.wav"

#define LEVEL_UP_SOUND  "LevelUp.wav"

#define STAR_CATCHER_SCORE_SOUND_1  "scoreg.wav"
#define STAR_CATCHER_SCORE_SOUND_2  "scorer.wav"

#define STAR_CATCH_PADDLE_RADIUS			2
#define STAR_GRAVITY						.003


uint32_t CurrentStarCatcherLevel;



        
typedef struct 
{
		float	X;
		float	Y;
		
		float 	Vx;
		float   Vy;	

		uint8_t	Bounces;
		uint8_t   Active;
		
} StarCatch;

#define MAX_STAR_CATCH_STARS	10

StarCatch MyStarCatchStars[MAX_STAR_CATCH_STARS];
uint8_t CurrentNumberGameStars;

uint8_t PaddleState;
uint32_t StarCatcherGameTime;

uint8_t StarCatcherState;
void StarCatcherProcess();
void ProcessStarCatcherGameTime();
void StarCatcherPreview();
void InitStarCatcherObjectsForGameplay();
void InitStarCatcherObjectsForPreview();
void DoStarCatcherGameLevelAdjust();
void StarCatcherPhysics();
void RenderStarCatchPaddles();
void StarCatcherRenderScores();
void MovePaddles();
void CheckForStarCatch();
uint8_t GetNumActiveStars();
void GenerateNewStar();
void  RenderStarCatcherGameTime();
void RenderStarCatcherObjects();
void ManageStarCatcherStars();


void Init__StarCatcher()
{

  uint32_t i = 0;
	
	Player.x =  eGFX_PHYSICAL_SCREEN_SIZE_X/3;

	Player.y = eGFX_PHYSICAL_SCREEN_SIZE_Y-1;
	
	STAR_CATCHER_RED_SCORE_DISPLAY_TIMER = 0xFFFF;
	STAR_CATCHER_GREEN_SCORE_DISPLAY_TIMER = 0xFFFF;
		StarCatcherGameTime = STAR_CATCHER_MAX_TIME;
        
        CurrentStarCatcherLevel = GREEN_LEVEL;
        
        STAR_CATCHER_SOUND_LOCKOUT_TIMER = 0;
        
        Player.Score = 0;

        CurrentNumberGameStars = 0;
   
   for(i=0;i<MAX_STAR_CATCH_STARS;i++)
   {
      MyStarCatchStars[i].Active = FALSE;
  
    
   }
 
 }

 


void Process__StarCatcher()
{
	if(STAR_CATCHER_GAME_LOGIC_TIMER>0)
		{
			STAR_CATCHER_GAME_LOGIC_TIMER = 0;
				
			eGFX_ImagePlane_Clear(&eGFX_BackBuffer);
 
			
           	 StarCatcherPhysics();
	 		 CheckForStarCatch();
	 		 MovePaddles();
	 		 RenderStarCatcherObjects();
	 		 ManageStarCatcherStars();
	 		 RenderStarCatcherGameTime();
	 		 ProcessStarCatcherGameTime();
	 		 DoStarCatcherGameLevelAdjust();
	 	     StarCatcherRenderScores();
           
            if( STARCATCHER_LEVELUP_TIMER < LEVEL_UP_MARQUEE_TIME)
                   {
                       eGFX_PutPixel(&eGFX_BackBuffer,MarqueePosition,0,CurrentStarCatcherLevel);
                     
                      eGFX_PutPixel(&eGFX_BackBuffer,eGFX_PHYSICAL_SCREEN_SIZE_X - 1 - MarqueePosition,
                                     eGFX_PHYSICAL_SCREEN_SIZE_Y-1,CurrentStarCatcherLevel);
                       eGFX_PutPixel(&eGFX_BackBuffer,0,MarqueePosition,CurrentStarCatcherLevel);
                       eGFX_PutPixel(&eGFX_BackBuffer,eGFX_PHYSICAL_SCREEN_SIZE_X-1,
                                     eGFX_PHYSICAL_SCREEN_SIZE_Y- 1 - MarqueePosition, CurrentStarCatcherLevel);
                       
                       MarqueePosition++;
                       if(MarqueePosition>=eGFX_PHYSICAL_SCREEN_SIZE_Y)
                        MarqueePosition = 0;
                       
                   }
           

            eGFX_Dump(&eGFX_BackBuffer);	
       }
}

void RenderStarCatcherGameTime()
{

}

void StarCatcherPhysics()
{
	uint8_t i;

	for(i=0;i<MAX_STAR_CATCH_STARS;i++)
	{
		if(MyStarCatchStars[i].Active == TRUE)
		{
				
				//move the stars
				MyStarCatchStars[i].X += MyStarCatchStars[i].Vx;
				MyStarCatchStars[i].Y += MyStarCatchStars[i].Vy;
				
				//acceleration due to gravity
				MyStarCatchStars[i].Vy += STAR_GRAVITY;
				
				//Check For Star catch
				CheckForStarCatch();
				
				//Process bouncing from the bottom of the screen
				if(MyStarCatchStars[i].Y>eGFX_PHYSICAL_SCREEN_SIZE_Y-1)
				{
					MyStarCatchStars[i].Y = (eGFX_PHYSICAL_SCREEN_SIZE_Y-1);
					//remove a little energy in each bounce
					MyStarCatchStars[i].Vy =  MyStarCatchStars[i].Vy * -0.95;
					MyStarCatchStars[i].Bounces++;
                    
					if(MyStarCatchStars[i].Bounces > BOUNCES_BEFORE_STAR_DESTRUCTION)
					{
						
						MyStarCatchStars[i].Active = FALSE;
						
					}
					else
					{
                                switch(rand()%5)
                                {
								default:
								case 0:
                                  
                                        playWav1.play(BOUNCE_0_SOUND);	
								break;
									
								case 1:
                              
									playWav1.play(BOUNCE_1_SOUND);	
								break;
								
								case 2:
                            
									playWav1.play(BOUNCE_2_SOUND);
								break;
								
								case 3:
                           
                                    playWav1.play(BOUNCE_3_SOUND);	
								break;
                                
                                case 4:
                             
                                    playWav1.play(BOUNCE_4_SOUND);	
                                break;
                                }
					}
						
				}
				
				
				//Process bouncing on the left and right sides of the screen
				if(MyStarCatchStars[i].X>eGFX_PHYSICAL_SCREEN_SIZE_X-1)
				{
					MyStarCatchStars[i].X=eGFX_PHYSICAL_SCREEN_SIZE_X-1;
					//remove a little energy in each bounce
						MyStarCatchStars[i].Vx = MyStarCatchStars[i].Vx * -1;
				}
				if(MyStarCatchStars[i].X<0)
				{
					MyStarCatchStars[i].X=0;
					MyStarCatchStars[i].Vx = MyStarCatchStars[i].Vx * -1;
				}
		}	
	}
	
}


void MovePaddles()
{
  
  
     UpdateUserInput();
    ProcessStandardInput();

Player.y =eGFX_PHYSICAL_SCREEN_SIZE_Y-1;
 
	if(Player.x<STAR_CATCH_PADDLE_RADIUS)
	{
		Player.x =STAR_CATCH_PADDLE_RADIUS;
	}
	
	if(Player.x >= (eGFX_PHYSICAL_SCREEN_SIZE_X-1-STAR_CATCH_PADDLE_RADIUS))
	{
		Player.x = eGFX_PHYSICAL_SCREEN_SIZE_X-1-STAR_CATCH_PADDLE_RADIUS;
		
	}
	
}




//This returns FALSE to indicate that there wasn't an intersection so we can check the other paddle
//it paddle top/bottom case
uint8_t Check_Star_RedPaddle_Intersecton(uint8_t i)
{
		if((MyStarCatchStars[i].X > (Player.x-STAR_CATCH_PADDLE_RADIUS-1)) 
		&& (MyStarCatchStars[i].X < (Player.x+STAR_CATCH_PADDLE_RADIUS+1))
		&& (MyStarCatchStars[i].Y >= Player.y))
			{
				MyStarCatchStars[i].Active = FALSE;
				Player.Score++;
			
				STAR_CATCHER_RED_SCORE_DISPLAY_TIMER = 0;
				
                
                if(Player.Score== YELLOW_LEVEL_THRESHOLD)
                {
                  playWav2.play(LEVEL_UP_SOUND);
                STAR_CATCHER_SOUND_LOCKOUT_TIMER = 0;
                  CurrentStarCatcherLevel = YELLOW_LEVEL;
                  STARCATCHER_LEVELUP_TIMER = 0;
                }
                else if (Player.Score== RED_LEVEL_THRESHOLD )
                {
                     CurrentStarCatcherLevel = RED_LEVEL;
                      playWav2.play(LEVEL_UP_SOUND); 
                        STARCATCHER_LEVELUP_TIMER = 0;
                      STAR_CATCHER_SOUND_LOCKOUT_TIMER = 0;
                }
                else if (Player.Score== BLUE_LEVEL_THRESHOLD)
                {
                      CurrentStarCatcherLevel = BLUE_LEVEL;
                       playWav2.play(LEVEL_UP_SOUND);
                       STAR_CATCHER_SOUND_LOCKOUT_TIMER = 0;
                         STARCATCHER_LEVELUP_TIMER = 0;
                }
                else if (Player.Score == PURPLE_LEVEL_THRESHOLD)
                {
                 CurrentStarCatcherLevel = PURPLE_LEVEL;
                  playWav2.play(LEVEL_UP_SOUND); 
                  STAR_CATCHER_SOUND_LOCKOUT_TIMER = 0;
                    STARCATCHER_LEVELUP_TIMER = 0;
                }
                else if (Player.Score == WHITE_LEVEL_THRESHOLD)
                {
                 CurrentStarCatcherLevel = WHITE_LEVEL;
                  playWav2.play(LEVEL_UP_SOUND); 
                  STAR_CATCHER_SOUND_LOCKOUT_TIMER = 0;
                    STARCATCHER_LEVELUP_TIMER = 0;
                }
                else
                {
                    if(rand()&0x01)
                    {
                        playWav2.play(STAR_CATCHER_SCORE_SOUND_1);
                        }
                    else
                    {
                        playWav2.play(STAR_CATCHER_SCORE_SOUND_2);     
                    }                        
                }    
                    
                return TRUE;
            }
            else
            {
                return FALSE;
                }
            
}

void CheckForStarCatch()
{
	uint32_t i;
	
	for(i=0;i<MAX_STAR_CATCH_STARS;i++)
	{
		if(MyStarCatchStars[i].Active == TRUE)
		{
			Check_Star_RedPaddle_Intersecton(i);
		}
	}
}



void RenderStarCatchPaddles()
{
	uint8_t Color;
    
    if(STAR_CATCHER_RED_SCORE_DISPLAY_TIMER < STAR_CATCHER_SCORE_DISPLAY_TIME)
    {
            eGFX_DrawHline(&eGFX_BackBuffer,
                     Player.x-STAR_CATCH_PADDLE_RADIUS,
                    Player.x+STAR_CATCH_PADDLE_RADIUS,
                    Player.y,
                    eGFX_PIXEL_RANDOM);
    }
    else
    {
    	  eGFX_DrawHline(&eGFX_BackBuffer,
                    Player.x-STAR_CATCH_PADDLE_RADIUS,
                    Player.x+STAR_CATCH_PADDLE_RADIUS,
                   Player.y,
                    CurrentStarCatcherLevel);
    }
    

	
}

void StarCatcherRenderScores()
{
}


void DoStarCatcherGameLevelAdjust()
{
	if(StarCatcherGameTime<	(STAR_CATCHER_MAX_TIME*1/5))
		CurrentNumberGameStars = 5;
	else if(StarCatcherGameTime<(STAR_CATCHER_MAX_TIME*2/5))
		CurrentNumberGameStars = 4;
    else if(StarCatcherGameTime<(STAR_CATCHER_MAX_TIME*3/5))
		CurrentNumberGameStars = 3;
	else if(StarCatcherGameTime<(STAR_CATCHER_MAX_TIME*4/5))
		CurrentNumberGameStars = 2;
	else
		CurrentNumberGameStars = 1;	
}

void ProcessStarCatcherGameTime()
{
	if(STAR_CATCHER_GAME_TIMER >= 100)
	{
		STAR_CATCHER_GAME_TIMER = 0;
		
		if(StarCatcherGameTime > 0)
		{
			StarCatcherGameTime--;		
		}
		else
		{
			  ShowScoreDisplay(CurrentStarCatcherLevel, Player.Score,"");
		}	
	}
}
void RenderStarCatcherObjects()
{
	uint32_t i;
	
	for(i=0;i<MAX_STAR_CATCH_STARS;i++)
	{
		if(MyStarCatchStars[i].Active == TRUE)
		{

			eGFX_PutPixel(&eGFX_BackBuffer,MyStarCatchStars[i].X,
										  MyStarCatchStars[i].Y,0xFF00FFFF);
		}
	}	

	RenderStarCatchPaddles();
}

void ManageStarCatcherStars()
{
  if(GetNumActiveStars() < CurrentNumberGameStars)
  {
  	GenerateNewStar();
  }
}

uint8_t GetNumActiveStars()
{
	uint8_t RetVal = 0;
	uint8_t i;
	
	for(i=0;i<MAX_STAR_CATCH_STARS;i++)
	{
		if((MyStarCatchStars[i].Active == TRUE))
		{
			RetVal++;			
		}
	}
	return RetVal;
}


void GenerateNewStar()
{
	uint8_t i;
	
	for(i=0;i<MAX_STAR_CATCH_STARS;i++)
	{
		if(MyStarCatchStars[i].Active == false)
		{
			MyStarCatchStars[i].Active = TRUE;
			MyStarCatchStars[i].Bounces = 0;
			
			if(rand()&0x01)
			{
				MyStarCatchStars[i].X =  rand()%eGFX_PHYSICAL_SCREEN_SIZE_X;
				MyStarCatchStars[i].Y =0;
				MyStarCatchStars[i].Vy = 0;
				MyStarCatchStars[i].Vx = -.09 * (float)rand()/(float)RAND_MAX - .05;
			}
			else
			{
				MyStarCatchStars[i].X = rand()%eGFX_PHYSICAL_SCREEN_SIZE_X;
				MyStarCatchStars[i].Y =0;
				MyStarCatchStars[i].Vy = 0;
				MyStarCatchStars[i].Vx =  .09 * (float)rand()/(float)RAND_MAX + 0.05;
			}	
				
			break;
		}	
		
	}	
}

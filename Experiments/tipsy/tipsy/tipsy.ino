#include "System.h"


void ChangeTipsyState(uint8_t NewState);
void adc0_timer_callback(void) ;
void playFile(const char *filename);

#define ENABLE_AUDIO




uint8_t TipsyState = TIPSY_STATE_INIT;
uint8_t SelectedGame = GAME_SELECTOR;
// hold elapsed time for a single sample to be converted and buffered
// Define the array of leds



int l;

//
// AUDIO STUFF
//
#ifdef ENABLE_AUDIO
// audio stuff
// GUItool: begin automatically generated code
//AudioPlaySdWav           playWav1;       //xy=154,78
//AudioOutputI2S           i2s1;           //xy=334,89
//AudioConnection          patchCord1(playWav1, 0, i2s1, 0);
//AudioConnection          patchCord2(playWav1, 1, i2s1, 1);
//AudioControlSGTL5000     sgtl5000_1;     //xy=240,153
// GUItool: end automatically generated code
#endif

// GUItool: begin automatically generated code
AudioPlaySdWav           playWav1;     //xy=430,134
AudioPlaySdWav           playWav2;     //xy=466,188
AudioMixer4              mixer1;         //xy=654,150
AudioOutputI2S           i2s1;           //xy=840,151
AudioConnection          patchCord1(playWav1, 0, mixer1, 0);
AudioConnection          patchCord2(playWav1, 1, mixer1, 1);
AudioConnection          patchCord3(playWav2, 0, mixer1, 2);
AudioConnection          patchCord4(playWav2, 1, mixer1, 3);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioConnection          patchCord6(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=494,302
// GUItool: end automatically generated code

//
// WIFI STUFF
//
#define WIFI_SERIAL Serial1
NeosCommMgr *ncm;
NeosCommMgr::GameDataCommon gdStatus;

uint32_t LastSwitch = 0;
uint32_t CurrentSwitch = 0;

void CheckSwitch()
{
  LastSwitch = CurrentSwitch;
  CurrentSwitch = digitalRead(SWITCH_PIN);

  if(LastSwitch == LOW && CurrentSwitch == HIGH)
  {
     ChangeTipsyState(TIPSY_RESET);
  }
}

void setup() 
{
  eGFX_InitDriver();

  Serial.begin(9600);
  Serial.println("tipsy");

  eGFX_ImagePlane_Clear(&eGFX_BackBuffer);
  eGFX_Blit(&eGFX_BackBuffer,
            0,
            0,
            &Sprite_32BPP_PW_Logo);
    
   eGFX_Dump(&eGFX_BackBuffer);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  #ifdef ENABLE_AUDIO
  AudioMemory(24);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.75);

  SPI.setMOSI(7);
  SPI.setSCK(14);
  if (!(SD.begin(10))) {
     Serial.println("Unable to access the SD card");
  }
  #endif
 
 mixer1.gain(0, 0.5);
 mixer1.gain(1, 0.5);
 mixer1.gain(2, 0.5);
 mixer1.gain(3, 0.5);
 
  delay(500);
  
  playWav1.play("cal.wav");
  
  delay(4000);
     
 // playWav1.play("cal.wav");
 
  delay(1000);
 
  InitUserInput();
 
  delay(1000);
  
  playWav1.play("ugb.wav");
 
  InitSystem();
     
  ChangeTipsyState(TIPSY_RESET);


  ncm = new NeosCommMgr(&WIFI_SERIAL);
  ncm->onConfigureGame
  (
    [] (NeosCommMgr::GameDataCommon *gameData)
    {
      
      Serial.println("onConfigureGame lambda");
      
    
      Serial.println("Setting Colors");
     // Player1Color = gameData->players[0].color;
      //Player2Color = gameData->players[1].color;
      
      ChangeTipsyState(TIPSY_RESET);
      
      Serial.print("New Game Started");
      
    }
  );
 

 
}


void loop()
{
  uint32_t i,j,k;
  float X,Y;

         
 
  CheckSwitch();
 
  /*
  ncm->process();
  
  if (sinceGameStatus > 250)
  {
    memset(&gdStatus, 0, sizeof(NeosCommMgr::GameDataCommon));
    
    gdStatus.players_len = MAX_NUM_PLAYERS;
    
    gdStatus.players[0].score  = 0;

    gdStatus.players[1].score  =  0;
    
    gdStatus.winner = GameVictor;
    
    ncm->sendGameStatus(&gdStatus);
    
    sinceGameStatus = 0;
  }*/
   
  switch(TipsyState)
  {
    default:
    case TIPSY_RESET:
 
        ChangeTipsyState(TIPSY_STATE_INIT);
 
    break;
    
    case TIPSY_STATE_INIT:

           ChangeTipsyState(TIPSY_STATE_PLAY);

           switch(SelectedGame)
           {
           
                case GAME_SELECTOR:
                
                    Init__GameSelector();
                
                break;
                
                case GAME_BUBBLE_POP:
                 Init__Game__BubblePop();
                 Serial.println("Starting Bubble Pop");
                break;
                
                case GAME_SCORE_DISPLAY:
                
                 Serial.println("Starting Score Display");
                
                break;
                
                case GAME_BUBBLE_MAZE:
                
                break;
                
                case GAME_STAR_CATCHER:
                    Init__StarCatcher();
                break;
                
                default:
                case GAME_INPUT_TEST:
                     Init__Game__SensorTest();
                break;
            }

           


    break;

       
    case TIPSY_STATE_PLAY:
     
         switch(SelectedGame)
           {
           
                case GAME_SELECTOR:
                
                    Process__Selector();
                
                break;
                
                case GAME_BUBBLE_POP:
                    Process__Game__BubblePop();
                break;
                
                case GAME_SCORE_DISPLAY:
                
                    ProcessScoreDisplay();
                    
                break;
                
                case GAME_BUBBLE_MAZE:
                
                break;
                
                case GAME_STAR_CATCHER:
                    Process__StarCatcher();
                break;
                
                default:
                case GAME_INPUT_TEST:
                        Process__Game__SensorTest();

                break;
            
           }

    break;
  
  
    case TIPSY_STATE_VICTORY:
    break;


    case TIPSY_STATE_DISPLAY_FLAG:
       



       break;
        
    
      if(l>0)
      {
        l= 0;
        digitalWrite(LED_FRAME_RATE,1);
      }
      else
      {
        l = 1;
        digitalWrite(LED_FRAME_RATE,0);
      }
     

   }
   
}


#ifdef ENABLE_AUDIO
void playFile(const char *filename)
{
    // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);

 
}
#endif


void ChangeTipsyState(uint8_t NewState)
{

  switch(NewState)
  {
      default:
    
      case TIPSY_RESET:

      TipsyState = NewState;
      
      break;

     
      case TIPSY_STATE_INIT:
       TipsyState = NewState;
      break;

      case TIPSY_STATE_PLAY:
        TipsyState = NewState;

      break;

      case TIPSY_STATE_VICTORY:
      

      TipsyState = NewState;

      break;


      case TIPSY_STATE_DISPLAY_FLAG:
  
        TipsyState = NewState;
      break;
  }
  
}


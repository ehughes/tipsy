#include <IntervalTimer.h>
#include <SerialFlash.h>
#include "FastLED.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <ADC.h>
#include "RingBuffer.h"

#define MAX_NUM_PLAYERS 2

#include <ArduinoJson.h>
#include <SerialPacketizer.h>
#include <NeosCommMgr.h>

void ChangeGameState(uint8_t NewState);
void adc0_timer_callback(void) ;
void PlotPixel_Player0(uint8_t Column,uint8_t Height, CRGB Color);
void PlotPixel_Player1(uint8_t Column,uint8_t Height, CRGB Color);
void RenderVoiceLevel_CheckTargets(uint8_t FillMode);
void DrawFillLevel();
void DrawTarget(uint8_t HighlightPlayer0,uint8_t HighlightPlayer1,uint8_t TargetMode);
void RegenTargets(uint8_t Mode);
 void playFile(const char *filename);

#define LEVEL_UP   "BOING.wav"
#define VICTORY     "SIREN.wav"
#define GAME_START "BELL.wav"
#define PLAYER_0 0
#define PLAYER_1 1


#define REGEN_TARGET_MODE_RANDOM      0
#define REGEN_TARGET_MODE_ALL_THE_WAY 1

#define FILL_MODE_IN_RADIUS 0
#define FILL_MODE_AMPLITUDE 1

#define FILL_MODE_AMPLITUDE_DIVISOR 5

#define ATTACK -.999f
#define DECAY  .999f
#define TARGET_HIT_RADIUS 3
#define ENABLE_AUDIO

#define LEDS_PER_STRING 84
#define NUM_LEDS (LEDS_PER_STRING)*12

#define DATA_PIN           2
#define CLOCK_PIN          5
#define LED_FRAME_RATE     4
#define SWITCH_PIN         3

#define GAME_RESET                      0
#define GAME_STATE_INIT                 1
#define GAME_STATE_PLAY                 2
#define GAME_STATE_CAPTURE_THE_FLAG     3
#define GAME_STATE_VICTORY              4
#define GAME_STATE_DISPLAY_FLAG         5
#define GAME_STATE_OFF                  6


uint8_t GameState = GAME_STATE_INIT;
int8_t GameVictor= - 1;

uint8_t Player1Target[6];
uint8_t Player0Target[6];
uint8_t Player0TargetIdx = 0;
uint8_t Player1TargetIdx = 0;
uint16_t  Pot[2];

float Env_State[2];

float Player0AmplitudeHistory[6];
uint8_t Player0HistoryIndex = 0;
uint8_t Player0FillLevel[6];
float Player1AmplitudeHistory[6];
uint8_t Player1HistoryIndex = 0;
uint8_t Player1FillLevel[6];

uint32_t Player0FillLevel_Fixed[6];
uint32_t Player1FillLevel_Fixed[6];

uint8_t Player0Color = 50;
uint8_t Player1Color = 128;


uint8_t  FlagOwner = PLAYER_0;
uint32_t FlagEffort = 0;
uint16_t FlagMajor;
uint16_t FlagMinor;

elapsedMillis gameTimer;
elapsedMicros sampleTime;
IntervalTimer adcTimer0;

ADC *adc = new ADC(); // adc object

// hold elapsed time for a single sample to be converted and buffered

uint16_t DAC_Out = 0;
uint8_t ADC0_ReadPhase;
uint8_t ADC1_ReadPhase;

// Define the array of leds
CRGB leds[NUM_LEDS];

uint8_t GlowIdx = 0;
uint8_t GlowTable[256] = {127, 130,  133,  136,  139,  143,  146,  149,  152,  155,  158,  161,  164,  167,  170,  173,
                          176,  179,  181,  184,  187,  190,  193,  195,  198,  200,  203,  205,  208,  210,  213,  215,
                          217,  219,  221,  223,  225,  227,  229,  231,  233,  235,  236,  238,  239,  241,  242,  243,
                          245,  246,  247,  248,  249,  250,  250,  251,  252,  252,  253,  253,  253,  254,  254,  254,
                          254,  254,  254,  254,  253,  253,  252,  252,  251,  251,  250,  249,  248,  247,  246,  245,
                          244,  243,  241,  240,  239,  237,  235,  234,  232,  230,  228,  226,  224,  222,  220,  218,
                          216,  214,  211,  209,  207,  204,  202,  199,  196,  194,  191,  188,  186,  183,  180,  177,
                          174,  171,  168,  166,  163,  159,  156,  153,  150,  147,  144,  141,  138,  135,  132,  129,
                          125,  122,  119,  116,  113,  110,  107,  104,  101,  98, 95, 91, 88, 86, 83, 80,
                          77, 74, 71, 68, 66, 63, 60, 58, 55, 52, 50, 47, 45, 43, 40, 38,
                          36, 34, 32, 30, 28, 26, 24, 22, 20, 19, 17, 15, 14, 13, 11, 10,
                          9,  8,  7,  6,  5,  4,  3,  3,  2,  2,  1,  1,  0,  0,  0,  0,
                          0,  0,  0,  1,  1,  1,  2,  2,  3,  4,  4,  5,  6,  7,  8,  9,
                          11, 12, 13, 15, 16, 18, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37,
                          39, 41, 44, 46, 49, 51, 54, 56, 59, 61, 64, 67, 70, 73, 75, 78,
                          81, 84, 87, 90, 93, 96, 99, 102,  105,  108,  111,  115,  118,  121,  124,  127};

int l;

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
     ChangeGameState(GAME_RESET);
  }

}

elapsedMillis sinceGameStatus;
#define HH_MAX_NUM_PLAYERS 2

void setup() 
{
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(4)>(leds, NUM_LEDS);

  Serial.begin(9600);
  delay(1000);
  Serial.println("Screamo");

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  #ifdef ENABLE_AUDIO
  AudioMemory(8);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.75);

  SPI.setMOSI(7);
  SPI.setSCK(14);
  if (!(SD.begin(10))) {
     Serial.println("Unable to access the SD card");
  }
  #endif


  adc->adc0->setReference(ADC_REF_3V3);
  adc->adc0->setAveraging(1); // set number of averages
  adc->adc0->setResolution(12); // set bits of resolution
  adc->adc0->setConversionSpeed(ADC_HIGH_SPEED); // change the conversion speed
  adc->adc0->setSamplingSpeed(ADC_HIGH_SPEED); // change the sampling speed

  adc->adc1->setReference(ADC_REF_3V3);
  adc->adc1->setAveraging(1); // set number of averages
  adc->adc1->setResolution(12); // set bits of resolution
  adc->adc1->setConversionSpeed(ADC_HIGH_SPEED); // change the conversion speed
  adc->adc1->setSamplingSpeed(ADC_HIGH_SPEED); // change the sampling speed
  
  adcTimer0.begin(adc0_timer_callback, 125);
 
  adc->enableInterrupts(ADC_0);
 
  analogWriteResolution(12);

  pinMode(LED_FRAME_RATE,OUTPUT);
  pinMode(SWITCH_PIN,INPUT_PULLUP);
  ChangeGameState(GAME_RESET);



  ncm = new NeosCommMgr(&WIFI_SERIAL);
  ncm->onConfigureGame
  (
    [] (NeosCommMgr::GameDataCommon *gameData)
    {
      
      Serial.println("onConfigureGame lambda");
      
    if(strncmp(gameData->mode,"StartGame",16) == 0)
      {
         Serial.println("Setting Colors");
         Player0Color = gameData->players[0].color;
         Player1Color = gameData->players[1].color;
         ChangeGameState(GAME_RESET);
         Serial.println("New Game Started");
      }

    if(strncmp(gameData->mode,"StopGame",16) == 0)
      {
         ChangeGameState(GAME_STATE_OFF);
         Serial.println("GAME_STATE_OFF");
      }
     
    }
  );
  
}



void loop()
{
  uint32_t i,j,k;
  float Gain1;
  float A;
   
  CRGB TmpColor;
         
  GlowIdx+=4;

  ncm->process();
  
  if (sinceGameStatus > 250)
  {
    
    memset(&gdStatus, 0, sizeof(NeosCommMgr::GameDataCommon));
    
    gdStatus.players_len = MAX_NUM_PLAYERS;
    

  
      gdStatus.players[0].score  =  ((float)Player0FillLevel[0]/(float)Player0Target[0]) * (100.0/6.0) +
                                    ((float)Player0FillLevel[1]/(float)Player0Target[1]) * (100.0/6.0) +
                                    ((float)Player0FillLevel[2]/(float)Player0Target[2]) * (100.0/6.0) + 
                                    ((float)Player0FillLevel[3]/(float)Player0Target[3]) * (100.0/6.0) +
                                    ((float)Player0FillLevel[4]/(float)Player0Target[4]) * (100.0/6.0) +
                                    ((float)Player0FillLevel[5]/(float)Player0Target[5]) * (100.0/6.0);

    gdStatus.players[1].score  =  ((float)Player1FillLevel[0]/(float)Player1Target[0]) * (100.0/6.0) +
                                    ((float)Player1FillLevel[1]/(float)Player1Target[1]) * (100.0/6.0) +
                                    ((float)Player1FillLevel[2]/(float)Player1Target[2]) * (100.0/6.0) + 
                                    ((float)Player1FillLevel[3]/(float)Player1Target[3]) * (100.0/6.0) +
                                    ((float)Player1FillLevel[4]/(float)Player1Target[4]) * (100.0/6.0) +
                                    ((float)Player1FillLevel[5]/(float)Player1Target[5]) * (100.0/6.0);
    
    
    gdStatus.winner = GameVictor;
    
    ncm->sendGameStatus(&gdStatus);
    sinceGameStatus = 0;
  }
 
  FastLED.clear();
   
  switch(GameState)
  {
    default:

    case GAME_RESET:
    CheckSwitch();

    Env_State[0] = 0;
    Env_State[1] = 0;
    FlagEffort = 0;
    gameTimer = 0;

    ChangeGameState(GAME_STATE_INIT);
    break;

    
    case GAME_STATE_INIT:

      RegenTargets(REGEN_TARGET_MODE_ALL_THE_WAY);
      ChangeGameState(GAME_STATE_PLAY);
      playWav1.play(GAME_START);

    break;

    case GAME_STATE_CAPTURE_THE_FLAG:

        CheckSwitch();

    break;

    case GAME_STATE_OFF:


    CheckSwitch();


    break;

    
    case GAME_STATE_PLAY:

      CheckSwitch();

       FastLED.clear();
 
       DrawTarget(Player0TargetIdx,Player1TargetIdx,REGEN_TARGET_MODE_ALL_THE_WAY);

       DrawFillLevel();
    
       RenderVoiceLevel_CheckTargets(FILL_MODE_AMPLITUDE);
   
    break;
  
  
    case GAME_STATE_VICTORY:

    FastLED.clear();
     
    if(gameTimer>3000)
    {
       ChangeGameState(GAME_STATE_DISPLAY_FLAG);

       
    }
    else
    {
      if(GameVictor == PLAYER_1)
      {
        FlagOwner = PLAYER_1;
        for(i=0;i<4;i++)
        {
          PlotPixel_Player0(rand()%6,rand()%LEDS_PER_STRING,CHSV(Player1Color,255,rand()));  
          PlotPixel_Player1(rand()%6,rand()%LEDS_PER_STRING,CHSV(Player1Color,255,rand()));  
         }
      }
      else
      {
        FlagOwner = PLAYER_0;
        for(i=0;i<4;i++)
        {
          PlotPixel_Player0(rand()%6,rand()%LEDS_PER_STRING,CHSV(Player0Color,255,rand()));  
          PlotPixel_Player1(rand()%6,rand()%LEDS_PER_STRING,CHSV(Player0Color,255,rand()));  
        }
        
      }
    }
     FastLED.show(); // Refresh strip
    break;


    case GAME_STATE_DISPLAY_FLAG:


    if(gameTimer>10000)
    {
       ChangeGameState(GAME_STATE_DISPLAY_FLAG);
    }
    
     CheckSwitch();

     FastLED.clear();

     if(GameVictor == PLAYER_1)
     {
       
        for(j=0;j<6;j++)
           {
                if(FlagMajor == 0)
                {
                  k =0;
                }
                else
                {
                  k  = FlagMajor+1;
                }
                  for(i=k;i<LEDS_PER_STRING/2;i++)
                    {
                          TmpColor = CHSV(Player1Color,255,(GlowTable[(GlowIdx + (j*16 + i))&0xFF]>>2) + 32);
                         
                          PlotPixel_Player0(j,LEDS_PER_STRING/2  + i, TmpColor);
                          PlotPixel_Player0(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
                          PlotPixel_Player1(j,LEDS_PER_STRING/2  + i, TmpColor);
                          PlotPixel_Player1(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
                      }
            }
        
     }

     
     if(GameVictor == PLAYER_0)
     {
     
        for(j=0;j<6;j++)
           {
                if(FlagMajor == 0)
                {
                  k =0;
                }
                else
                {
                  k  = FlagMajor+1;
                }
                  for(i=k;i<LEDS_PER_STRING/2;i++)
                    {
                          TmpColor = CHSV(Player0Color,255,(GlowTable[(GlowIdx + (j*16 + i))&0xFF]>>2) + 32);
                          
                          PlotPixel_Player0(j,LEDS_PER_STRING/2  + i, TmpColor);
                          PlotPixel_Player0(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
                          PlotPixel_Player1(j,LEDS_PER_STRING/2  + i, TmpColor);
                          PlotPixel_Player1(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
                    }
        
         }
     }

    break;
    
  }

   Pot[0] = adc->analogRead(A2, ADC_1);
   Pot[1] = adc->analogRead(A3, ADC_1);



  //To see the frame rate

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
  
  FastLED.show(); // Refresh strip
  
}


#ifdef ENABLE_AUDIO
void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);

  // A brief delay for the library read WAV info
  delay(5);

  //AudioProcessorUsageMaxReset();
  // Simply wait for the file to finish playing.
 // while (playWav1.isPlaying()) {
   // updateleds();
  //}
  //Serial.println(AudioProcessorUsageMax());
}
#endif




void adc0_timer_callback(void) 
{
  ADC0_ReadPhase++;
  ADC0_ReadPhase &= 0x01;

  if(ADC0_ReadPhase == 0)
      adc->startSingleRead(A6, ADC_0);
  else
      adc->startSingleRead(A7 , ADC_0);
}



void adc0_isr()
{

   uint16_t e;
   float  tmp;

   e = (adc->readSingle());
   
   tmp = (float)abs(e - 2048);

 
   if(ADC0_ReadPhase == 0)
   {
     
    
        Env_State[0] =tmp +  ATTACK * (float)(tmp - Env_State[0]);
     
        analogWrite(A14, Env_State[0]);     
   }
   else
    {

      tmp -= 80;
      if(tmp<0)
        tmp = 0;
             Env_State[1] = tmp +  ATTACK * (float)(tmp - Env_State[1]);
    }     
}


void PlotPixel_Player0(uint8_t Column,uint8_t Height, CRGB Color)
{
  if(Height >= LEDS_PER_STRING)
    Height = LEDS_PER_STRING;

  if(Column>5)
    Column = 5;
      
  switch(Column)
  {
    default:
    case 0:

        leds[Height] = Color;
    
    break;
    
    case 1:

        leds[3*LEDS_PER_STRING + (LEDS_PER_STRING - 1 - Height)] = Color;
    
    break;
    
    case 2:

         leds[4*LEDS_PER_STRING + (Height)] = Color;
    
    break;
    
    case 3:
    
       leds[7*LEDS_PER_STRING + (LEDS_PER_STRING - 1 - Height)] = Color;
    
    break;
    
    case 4:

      leds[8*LEDS_PER_STRING + (Height)] = Color;
    
    
    break;
    
    case 5:

        leds[11*LEDS_PER_STRING + (LEDS_PER_STRING - 1 - Height)] = Color;
   
    break;
    
  }
  
}

void PlotPixel_Player1(uint8_t Column,uint8_t Height, CRGB Color)
{
  if(Height >= LEDS_PER_STRING)
    Height = LEDS_PER_STRING;

  if(Column>5)
    Column = 5;
      
  switch(Column)
  {
    default:
    case 0:

       leds[1*LEDS_PER_STRING + (LEDS_PER_STRING - 1 - Height)] = Color;
    
    break;
    
    case 1:

        leds[2*LEDS_PER_STRING + (Height)] = Color;
    
    break;
    
    case 2:

         leds[5*LEDS_PER_STRING + (LEDS_PER_STRING - 1 - Height)] = Color;
    
    break;
    
    case 3:
    
      leds[6*LEDS_PER_STRING + (Height)] = Color;
    
    break;
    
    case 4:

       leds[9*LEDS_PER_STRING + (LEDS_PER_STRING - 1 - Height)] = Color;
    
    
    break;
    
    case 5:

        leds[10*LEDS_PER_STRING + (Height)] = Color;
   
    break;
    
  }
  
}

void DrawTarget(uint8_t HighlightPlayer0,uint8_t HighlightPlayer1,uint8_t TargetMode)
{
  uint8_t i;

  for(i=0;i<6;i++)
  {

    if(TargetMode == REGEN_TARGET_MODE_RANDOM)
    {
         if(i != HighlightPlayer0)
          {
            PlotPixel_Player0(i,Player0Target[i]-2,CHSV(Player0Color, 255, 255));
            PlotPixel_Player0(i,Player0Target[i]-1,CHSV(Player0Color, 255, 255));
            PlotPixel_Player0(i,Player0Target[i],CHSV(Player0Color, 255, 255));
            PlotPixel_Player0(i,Player0Target[i]+1,CHSV(Player0Color, 255, 255));
            PlotPixel_Player0(i,Player0Target[i]+2,CHSV(Player0Color, 255, 255));
          }
          else
          {
            PlotPixel_Player0(i,Player0Target[i]-2,CHSV(Player0Color, 255, GlowTable[GlowIdx]));
            PlotPixel_Player0(i,Player0Target[i]-1,CHSV(Player0Color, 255, GlowTable[(GlowIdx+51)&0xff]));
            PlotPixel_Player0(i,Player0Target[i],CHSV(Player0Color, 255, GlowTable[(GlowIdx+102)&0xff]));
            PlotPixel_Player0(i,Player0Target[i]+1,CHSV(Player0Color, 255, GlowTable[(GlowIdx+102+51)&0xff]));
            PlotPixel_Player0(i,Player0Target[i]+2,CHSV(Player0Color, 255, GlowTable[(GlowIdx+102+51+51)&0xff]));
      
          }
    }
    else
    {
      PlotPixel_Player0(i,Player0Target[i]-2,CHSV(Player0Color, 255, GlowTable[GlowIdx]));
      PlotPixel_Player0(i,Player0Target[i]-1,CHSV(Player0Color, 255, GlowTable[(GlowIdx+51)&0xff]));
      PlotPixel_Player0(i,Player0Target[i],CHSV(Player0Color, 255, GlowTable[(GlowIdx+102)&0xff]));
      PlotPixel_Player0(i,Player0Target[i]+1,CHSV(Player0Color, 255, GlowTable[(GlowIdx+102+51)&0xff]));
      PlotPixel_Player0(i,Player0Target[i]+2,CHSV(Player0Color, 255, GlowTable[(GlowIdx+102+51+51)&0xff]));
    }
  }
  

  for(i=0;i<6;i++)
  {

    if(TargetMode == REGEN_TARGET_MODE_RANDOM)
    {
         if(i != HighlightPlayer1)
          {
            PlotPixel_Player1(i,Player0Target[i]-2,CHSV(Player1Color, 255, 255));
            PlotPixel_Player1(i,Player0Target[i]-1,CHSV(Player1Color, 255, 255));
            PlotPixel_Player1(i,Player0Target[i],CHSV(Player1Color, 255, 255));
            PlotPixel_Player1(i,Player0Target[i]+1,CHSV(Player1Color, 255, 255));
            PlotPixel_Player1(i,Player0Target[i]+2,CHSV(Player1Color, 255, 255));
          }
          else
          {
            PlotPixel_Player1(i,Player0Target[i]-2,CHSV(Player1Color, 255, GlowTable[GlowIdx]));
            PlotPixel_Player1(i,Player0Target[i]-1,CHSV(Player1Color, 255, GlowTable[(GlowIdx+51)&0xff]));
            PlotPixel_Player1(i,Player0Target[i],CHSV(Player1Color, 255, GlowTable[(GlowIdx+102)&0xff]));
            PlotPixel_Player1(i,Player0Target[i]+1,CHSV(Player1Color, 255, GlowTable[(GlowIdx+102+51)&0xff]));
            PlotPixel_Player1(i,Player0Target[i]+2,CHSV(Player1Color, 255, GlowTable[(GlowIdx+102+51+51)&0xff]));
      
          }
    }
    else
    {
      PlotPixel_Player1(i,Player1Target[i]-2,CHSV(Player1Color, 255, GlowTable[GlowIdx]));
      PlotPixel_Player1(i,Player1Target[i]-1,CHSV(Player1Color, 255, GlowTable[(GlowIdx+51)&0xff]));
      PlotPixel_Player1(i,Player1Target[i],CHSV(Player1Color, 255, GlowTable[(GlowIdx+102)&0xff]));
      PlotPixel_Player1(i,Player1Target[i]+1,CHSV(Player1Color, 255, GlowTable[(GlowIdx+102+51)&0xff]));
      PlotPixel_Player1(i,Player1Target[i]+2,CHSV(Player1Color, 255, GlowTable[(GlowIdx+102+51+51)&0xff]));
    }
  }
 
}



void RegenTargets(uint8_t Mode)
{
  uint8_t i;

  switch(Mode)
  {
    default:
    case REGEN_TARGET_MODE_RANDOM:

    for(i=0;i<6;i++)
      {
          Player0Target[i] = rand()%50+20;
          Player1Target[i] = Player0Target[i];
      }


    break;

    case REGEN_TARGET_MODE_ALL_THE_WAY:
    
    
    for(i=0;i<6;i++)
     {
      Player0Target[i] = 84-TARGET_HIT_RADIUS;
      Player1Target[i] = 84-TARGET_HIT_RADIUS;
     }

    break;
  }
  
  
  //Reset the Fill Levels
  for(i=0;i<6;i++)
  {
    Player0FillLevel[i] = 0;
    Player1FillLevel[i] = 0;
        Player0FillLevel_Fixed[i] = 0;
    Player1FillLevel_Fixed[i] = 0;
  }

  Player0TargetIdx = 5;
  Player1TargetIdx = 5 ;
  
}


void DrawFillLevel()
{
uint8_t i,j;

  for(i=0;i<6;i++)
  {
    for(j=0;j<Player0FillLevel[i];j++)
    {
        PlotPixel_Player0(i,j,CHSV(Player0Color,255,32));
    }

    if(Player1FillLevel[i]>0)
       PlotPixel_Player0(i,Player1FillLevel[i],CHSV(Player1Color,255,32));
    
    
    for(j=0;j<Player1FillLevel[i];j++)
    {
        PlotPixel_Player1(i,j,CHSV(Player1Color,255,32));
    }

    if(Player0FillLevel[i]>0)
       PlotPixel_Player1(i,Player0FillLevel[i],CHSV(Player0Color,255,32));
  }
}


void RenderVoiceLevel_CheckTargets(uint8_t FillMode)
{
  uint32_t i,j,A;

  float Gain1 = ((Pot[0] / 4096.0) * 2.0) + 0.5;
  float Gain2 = ((Pot[1] / 4096.0) * 2.0) + 0.5;

    A = (int)(((float)Env_State[0] / (float)2048) *  Gain1  * 83.0);
    if(A>83)
      A = 83;


    Player0AmplitudeHistory[Player0HistoryIndex] = A;
    Player0HistoryIndex++;
    if(Player0HistoryIndex>5)
      Player0HistoryIndex = 0;

     j = Player0HistoryIndex;
    
    for(i=0;i<6;i++)
    {
      
      if(j == 0)
        j = 5;
      else
        j--;
        
      PlotPixel_Player0(0,Player0AmplitudeHistory[5-i],CHSV(Player0Color,255,255-(i*10)));  
      PlotPixel_Player0(1,Player0AmplitudeHistory[5-i],CHSV(Player0Color,255,255-(i*10)));  
      PlotPixel_Player0(2,Player0AmplitudeHistory[5-i],CHSV(Player0Color,255,255-(i*10)));  
      PlotPixel_Player0(3,Player0AmplitudeHistory[5-i],CHSV(Player0Color,255,255-(i*10)));  
      PlotPixel_Player0(4,Player0AmplitudeHistory[5-i],CHSV(Player0Color,255,255-(i*10)));  
      PlotPixel_Player0(5,Player0AmplitudeHistory[5-i],CHSV(Player0Color,255,255-(i*10)));  
      
    }


      if(FillMode == FILL_MODE_IN_RADIUS)
      {
        if((abs((int)A - (int)Player0Target[Player0TargetIdx])<TARGET_HIT_RADIUS))
        {
          Player0FillLevel[Player0TargetIdx]++;
         
        }
      }
      else
      {
        
        Player0FillLevel_Fixed[Player0TargetIdx] += A;
        Player0FillLevel[Player0TargetIdx] = Player0FillLevel_Fixed[Player0TargetIdx]>>FILL_MODE_AMPLITUDE_DIVISOR;     
      }

    if(Player0FillLevel[Player0TargetIdx]>= Player0Target[Player0TargetIdx])
    {
        Player0FillLevel[Player0TargetIdx] = Player0Target[Player0TargetIdx];

        playFile(LEVEL_UP);

        if(Player0TargetIdx>0)
        {
          Player0TargetIdx--;
          gameTimer = 0;
        }
         else
        {
          GameVictor = PLAYER_0;
          ChangeGameState(GAME_STATE_VICTORY);
        }
        
    }

 
    A = (int)(((float)Env_State[1] / (float)2048) *  Gain2  * 83.0);
    if(A>83)
      A = 83;

      
    Player1AmplitudeHistory[Player1HistoryIndex] = A;
    Player1HistoryIndex++;
    if(Player1HistoryIndex>5)
      Player1HistoryIndex = 0;

     j = Player1HistoryIndex;
    
    for(i=0;i<6;i++)
    {
      
      if(j == 0)
        j = 5;
      else
        j--;
        
      PlotPixel_Player1(0,Player1AmplitudeHistory[5-i],CHSV(Player1Color,255,255-(i*10)));  
      PlotPixel_Player1(1,Player1AmplitudeHistory[5-i],CHSV(Player1Color,255,255-(i*10)));  
      PlotPixel_Player1(2,Player1AmplitudeHistory[5-i],CHSV(Player1Color,255,255-(i*10)));  
      PlotPixel_Player1(3,Player1AmplitudeHistory[5-i],CHSV(Player1Color,255,255-(i*10)));  
      PlotPixel_Player1(4,Player1AmplitudeHistory[5-i],CHSV(Player1Color,255,255-(i*10)));  
      PlotPixel_Player1(5,Player1AmplitudeHistory[5-i],CHSV(Player1Color,255,255-(i*10)));  
      
    }

    if(FillMode == FILL_MODE_IN_RADIUS)
      {
        if((abs((int)A - (int)Player1Target[Player1TargetIdx])<TARGET_HIT_RADIUS))
          {
            Player1FillLevel[Player1TargetIdx]++;
            gameTimer = 0;
          }
      }
      else
      {
        
        Player1FillLevel_Fixed[Player1TargetIdx] += A;

        Player1FillLevel[Player1TargetIdx] = Player1FillLevel_Fixed[Player1TargetIdx]>>FILL_MODE_AMPLITUDE_DIVISOR;

      }
 if(Player1FillLevel[Player1TargetIdx]>= Player1Target[Player1TargetIdx])
    {
        Player1FillLevel[Player1TargetIdx] = Player1Target[Player1TargetIdx];

        playFile(LEVEL_UP);

        if(Player1TargetIdx>0)
          Player1TargetIdx--;
        else
        {
          GameVictor = PLAYER_1;
          ChangeGameState(GAME_STATE_VICTORY);
        }
    }  
}

void ChangeGameState(uint8_t NewState)
{

  switch(NewState)
  {

      case GAME_STATE_CAPTURE_THE_FLAG:

      GameState = NewState;
      

      break;
      
      case GAME_RESET:

      GameState = NewState;
      
      break;

      case GAME_STATE_OFF:

      
          GameState = NewState;
         FastLED.show(); // Refresh strip
         FastLED.clear();

         
      break;
      
      default:
      case GAME_STATE_INIT:

       GameState = NewState;
       GameVictor = -1;
      break;

      case GAME_STATE_PLAY:

      GameState = NewState;

      gameTimer = 0;
      GameVictor = -1;
      
      break;

      case GAME_STATE_VICTORY:
      
      gameTimer = 0;
      GameState = NewState;

      if(GameVictor == PLAYER_0)
        Serial.println("PLAYER_0 Wins");
      else
        Serial.println("PLAYER_1 Wins");
        
      playFile(VICTORY);
      break;



      case GAME_STATE_DISPLAY_FLAG:

        gameTimer = 0;
        GameState = NewState;

      break;
      
  }
  
}

/*

      CheckSwitch();

     FastLED.clear();

    if(FlagOwner == PLAYER_0)                                                                                                                                                                                                                                                     
    {

      Gain1 = ((Pot[0] / 4096.0) * 2.0) + 0.5;
       A = (int)(((float)Env_State[0] * Gain1));
     
      if(A > 100)
        FlagEffort+=((int)A)>>5;
      else
      {
        if(FlagEffort>64)
          FlagEffort-=64;
         else
         {
           FlagEffort = 0;
         }
      }
    }
    else
    {

      Gain1 = ((Pot[1] / 4096.0) * 2.0) + 0.5;
      A = (int)(((float)Env_State[1] * Gain1));
       
     if(A > 100)
        FlagEffort+=((int)A)>>5;
      else
      {
        if(FlagEffort>64)
          FlagEffort-=64;
         else
         {
           FlagEffort = 0;
         }
      }
      
    }
    
     FlagMajor = FlagEffort>>8;
     FlagMinor = (FlagEffort&0xFF);


      if(FlagMajor >= 84/2)
      {
        FlagEffort = 0;
        
        if(FlagOwner == PLAYER_0)
             {
                  FlagOwner = PLAYER_1;
             }
        else
          {
              FlagOwner = PLAYER_0;
          }
          break;
      }

     if(FlagMajor >= LEDS_PER_STRING/2)
     {
        FlagMajor = LEDS_PER_STRING/2;
     }
    

     if(FlagOwner == PLAYER_0)
     {
       
        TmpColor.r = 0;
        TmpColor.b = 32;
        TmpColor.g = 0;

        for(i=0;i<FlagMajor;i++)
        {
            for(j=0;j<6;j++)
            {
              PlotPixel_Player0(j,LEDS_PER_STRING/2  + i, TmpColor);
              PlotPixel_Player0(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
               PlotPixel_Player1(j,LEDS_PER_STRING/2  + i, TmpColor);
              PlotPixel_Player1(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
            }
        }
       
        TmpColor.r =  32 -(FlagMinor>>3);
        TmpColor.b = (FlagMinor)>>3;
        
          for(j=0;j<6;j++)
          {
           PlotPixel_Player0(j,LEDS_PER_STRING/2 +  FlagMajor , TmpColor);
           PlotPixel_Player0(j,LEDS_PER_STRING/2 -  FlagMajor-1, TmpColor);
           PlotPixel_Player1(j,LEDS_PER_STRING/2 +  FlagMajor , TmpColor);
           PlotPixel_Player1(j,LEDS_PER_STRING/2 -  FlagMajor-1, TmpColor);
          }



        for(j=0;j<6;j++)
           {
             

                if(FlagMajor == 0)
                {
                  k =0;
                }
                else
                {
                  k  = FlagMajor+1;
                }
                  for(i=k;i<LEDS_PER_STRING/2;i++)
                    {
                        TmpColor.r = GlowTable[(GlowIdx + (j*16 + i))&0xFF]>>3;
                            TmpColor.g = 0;
                            TmpColor.b = 0;
                          PlotPixel_Player0(j,LEDS_PER_STRING/2  + i, TmpColor);
                          PlotPixel_Player0(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
                          PlotPixel_Player1(j,LEDS_PER_STRING/2  + i, TmpColor);
                          PlotPixel_Player1(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
                      }
            }
        
     }

     
     if(FlagOwner == PLAYER_1)
     {
       
        TmpColor.g = 0;
        TmpColor.r = 32;
        TmpColor.b = 0;

        for(i=0;i<FlagMajor;i++)
        {
            for(j=0;j<6;j++)
            {
              PlotPixel_Player0(j,LEDS_PER_STRING/2  + i, TmpColor);
              PlotPixel_Player0(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
               PlotPixel_Player1(j,LEDS_PER_STRING/2  + i, TmpColor);
              PlotPixel_Player1(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
            }
        }
       
        TmpColor.b =  32 -(FlagMinor>>3);
        TmpColor.r = (FlagMinor)>>3;
        
          for(j=0;j<6;j++)
          {
           PlotPixel_Player0(j,LEDS_PER_STRING/2 +  FlagMajor , TmpColor);
           PlotPixel_Player0(j,LEDS_PER_STRING/2 -  FlagMajor-1, TmpColor);
           PlotPixel_Player1(j,LEDS_PER_STRING/2 +  FlagMajor , TmpColor);
           PlotPixel_Player1(j,LEDS_PER_STRING/2 -  FlagMajor-1, TmpColor);
          }


        for(j=0;j<6;j++)
           {
      

                if(FlagMajor == 0)
                {
                  k =0;
                }
                else
                {
                  k  = FlagMajor+1;
                }
                  for(i=k;i<LEDS_PER_STRING/2;i++)
                    {
                           TmpColor.b = GlowTable[(GlowIdx + (j*16 + i))&0xFF]>>3;
                           TmpColor.r = 0;
                           TmpColor.g = 0;
                          
                          PlotPixel_Player0(j,LEDS_PER_STRING/2  + i, TmpColor);
                          PlotPixel_Player0(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
                          PlotPixel_Player1(j,LEDS_PER_STRING/2  + i, TmpColor);
                          PlotPixel_Player2(j,LEDS_PER_STRING/2 - i - 1, TmpColor);
                    }
        
         }
     }

    FastLED.show();
    
    break;
*/

                                                                                                   


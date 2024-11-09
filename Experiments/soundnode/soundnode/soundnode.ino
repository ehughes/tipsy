#include <IntervalTimer.h>
#include <SerialFlash.h>
#include "FastLED.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <ADC.h>
#include "RingBuffer.h"



#include <ArduinoJson.h>
#include <SerialPacketizer.h>
#include <NeosCommMgr.h>


#define ENABLE_AUDIO

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

elapsedMillis sinceGameStatus;

char FileNameString[256];

void setup() 
{

  Serial.begin(9600);
  delay(1000);
  Serial.println("Soundnode");

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

  ncm = new NeosCommMgr(&WIFI_SERIAL);
  ncm->onConfigureGame
  (
    [&] (NeosCommMgr::GameDataCommon *gameData)
    {
      
      Serial.println("onConfigureGame lambda");
      
      if(strncmp(gameData->mode,"PlayAudio",16) == 0)
      {
        strncpy(FileNameString,gameData->state,256);
          playWav1.play(FileNameString);
          delay(5);
         //Serial.println("PL
      }

      if(strncmp(gameData->mode,"KillAudio",16) == 0)
      {
        playWav1.stop();
       }
      
      
    }
  );

}



void loop()
{

 
  ncm->process();
  
  if (sinceGameStatus > 250)
  {
    
    memset(&gdStatus, 0, sizeof(NeosCommMgr::GameDataCommon));
    
  
    ncm->sendGameStatus(&gdStatus);
    sinceGameStatus = 0;
  }
 
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




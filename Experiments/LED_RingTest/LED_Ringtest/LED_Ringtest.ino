#include <IntervalTimer.h>
#include <SerialFlash.h>
#include "FastLED.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <ADC.h>

#define NUM_LEDS      32



CRGB leds[NUM_LEDS];
#define DATA_PIN 5
#define CLK_PIN 2


void setup() 
{
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(1)>(leds, NUM_LEDS);

  Serial.begin(9600);
  Serial.println("LED Test");

  
}

uint16_t LED_Pos = 0;
uint32_t e=0;

void loop()
{
  uint32_t i,j,k;

     
     //eGFX_ImagePlane_Clear(&eGFX_BackBuffer);
     //eGFX_DrawLine(&eGFX_BackBuffer,0,0,eGFX_PHYSICAL_SCREEN_SIZE_X-1,eGFX_PHYSICAL_SCREEN_SIZE_Y-1, 0xFF);
     //eGFX_Dump(&eGFX_BackBuffer);
     
       LED_Pos++;
    if(LED_Pos>= NUM_LEDS)
        LED_Pos = 0;
    
    leds[LED_Pos].r = 0;
    leds[LED_Pos].g = 255;
    leds[LED_Pos].b = 0;
    
    leds[NUM_LEDS-1-LED_Pos].r = 0;
    leds[NUM_LEDS-1-LED_Pos].g = 0;
    leds[NUM_LEDS-1-LED_Pos].b = 255;
    
    
      //  if(gameTimer>60*1000)
       // {
        //  ChangeGameState(GAME_STATE_INIT);
       // }
        
        FastLED.show();

}




#include "eGFX_DataTypes.h"
#include "eGFX.h"
#include "eGFX_Driver_APA102_28_28.h"
#include "TipsyConfig.h"
#include "System.h"

eGFX_ImagePlane eGFX_BackBuffer;

#define DATA_PIN          PTD7
#define CLOCK_PIN         PTD0

#define DATA_PIN_HIGH   GPIOD_PSOR = 1<<7
#define DATA_PIN_LOW    GPIOD_PCOR = 1<<7

#define CLK_PIN_HIGH   GPIOD_PSOR = 1<<0
#define CLK_PIN_LOW    GPIOD_PCOR = 1<<0

#define HALF_CLK_DELAY(x)__asm("nop"); __asm("nop");//for(d=0;d<x;d++){c++;}

volatile int32_t c;
volatile int32_t d;

void WriteAPA102_Word(uint32_t Data)
{
    uint32_t i;
    
    for(i=0;i<32;i++)
    {
        if(Data & 0x80000000)
            DATA_PIN_HIGH;
        else    
            DATA_PIN_LOW;
        
        Data = Data << 1;
        
        HALF_CLK_DELAY(0);
        CLK_PIN_HIGH;
        HALF_CLK_DELAY(0);
        HALF_CLK_DELAY(0);
        CLK_PIN_LOW;
        HALF_CLK_DELAY(0);
   }

}

void WriteAPA102_EOF(uint32_t Clocks)
{
    uint32_t i; 
    
     DATA_PIN_HIGH;
   
   for(i=0;i<Clocks;i++)
   {
            HALF_CLK_DELAY(0);
            CLK_PIN_HIGH;
            HALF_CLK_DELAY(0);
            HALF_CLK_DELAY(0);
            CLK_PIN_LOW;
            HALF_CLK_DELAY(0);
   }
}





uint8_t BackBufferStore[eGFX_CALCULATE_32BPP_IMAGE_STORAGE_SPACE_SIZE(eGFX_PHYSICAL_SCREEN_SIZE_X,eGFX_PHYSICAL_SCREEN_SIZE_Y)];

void eGFX_InitDriver()
{
    int i;
    
    eGFX_ImagePlaneInit(&eGFX_BackBuffer,(uint8_t *)&BackBufferStore[0],
                        eGFX_PHYSICAL_SCREEN_SIZE_X, eGFX_PHYSICAL_SCREEN_SIZE_Y,
                        eGFX_IMAGE_PLANE_32BPP);
    //FastLED.clear();
    PORTD_PCR0 = PORT_PCR_MUX(1) | PORT_PCR_DSE;
    PORTD_PCR7 = PORT_PCR_MUX(1)| PORT_PCR_DSE;
    GPIOD_PDDR |= (1<<0) | (1<<7);
    GPIOD_PCOR = (1<<0) | (1 <<7);
      
   
     
    for(i=0;i<28*28*2;i++)
    {    
      WriteAPA102_Word(0x0000000);
    }
    for(i=0;i<28*28*2;i++)
    {    
      WriteAPA102_Word(0xFF00000);
    }
       
 }
uint32_t qq =0;
void eGFX_Dump(eGFX_ImagePlane *Image)
{
    uint32_t x,y,c;

  // WriteAPA102_Word(0x00);
  // WriteAPA102_Word(0x00);
  // WriteAPA102_Word(0x00);
    
    WriteAPA102_Word(0x0000000);
      
    for(x=0;x<eGFX_PHYSICAL_SCREEN_SIZE_X;x++)
    {
        for(y=0;y<eGFX_PHYSICAL_SCREEN_SIZE_X;y++)
        {
     
            if(x&0x01)
            {
               c = eGFX_GetPixel(Image,x,y);
            }
           else
            {
                c = eGFX_GetPixel(Image,x,eGFX_PHYSICAL_SCREEN_SIZE_Y - 1 - y);
            }
            
          
                WriteAPA102_Word(c | 0xFF000000);

        }
    }
    
     WriteAPA102_EOF(28*28*8/2);
}

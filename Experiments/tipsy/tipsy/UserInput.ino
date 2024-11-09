#include "System.h"

#define ACCEL_BUF_SIZE 32

volatile int32_t AccelX_Raw = 2048;
volatile int32_t AccelY_Raw = 2048;
volatile int32_t AccelZ_Raw = 2048;
volatile int32_t PotRaw = 2048;

volatile int32_t AccelRaw_Ready = FALSE;
volatile int32_t AccelX_Zero = 2048;
volatile int32_t AccelY_Zero = 2048;
volatile int32_t AccelZ_Zero = 2048;
volatile int32_t AccelX_Filtered = 0;
volatile int32_t AccelY_Filtered = 0;
volatile int32_t AccelZ_Filtered = 0;

int32_t GameBoard_X = 0;
int32_t GameBoard_Y = 0;




volatile int32_t AccelBuff_X[ACCEL_BUF_SIZE];
volatile int32_t AccelBuff_Y[ACCEL_BUF_SIZE];
volatile int32_t AccelBuffIdx;
uint32_t ADC0_ReadPhase = 0;

volatile uint32_t AccelX_RawAvg[256];


ADC *adc = new ADC(); // adc object
IntervalTimer adcTimer0;


void InitUserInput()
{


  adc->adc0->setReference(ADC_REF_3V3);
  adc->adc0->setAveraging(16); // set number of averages
  adc->adc0->setResolution(12);// set bits of resolution
  adc->adc0->setConversionSpeed(ADC_HIGH_SPEED);// change the conversion speed
  adc->adc0->setSamplingSpeed(ADC_HIGH_SPEED); // change the sampling speed
/*
  adc->adc1->setReference(ADC_REF_3V3);
  adc->adc1->setAveraging(1); // set number of averages
  adc->adc1->setResolution(12); // set bits of resolution
  adc->adc1->setConversionSpeed(ADC_HIGH_SPEED); // change the conversion speed
  adc->adc1->setSamplingSpeed(ADC_HIGH_SPEED); // change the sampling speed
  */
  
  adcTimer0.begin(adc0_timer_callback,2000);
 
  adc->enableInterrupts(ADC_0);


  ZeroTiltSensors();

}

void ZeroTiltSensors()
{
	uint32_t i = 0;
	
      Serial.println("Skipping 1st 256 samples");

     for(i = 0; i< 256 ; i++)
        {
            while(AccelRaw_Ready == 0){}
            
             AccelRaw_Ready = 0;
        }
      
       Serial.println("Finding the Zero Point");
       
       AccelX_Zero = 0;
       AccelY_Zero = 0;
  //     AccelZ_Zero = AccelZ_Zero;
       
        for(i = 0; i< 256 ; i++)
        {
            while(AccelRaw_Ready == 0)
            {
            }
                  AccelRaw_Ready = 0;
                  AccelX_Zero += AccelX_Raw;
                  AccelY_Zero += AccelY_Raw;
                 // AccelZ_Zero += AccelZ_Raw;  
        }
        
     AccelX_Zero = AccelX_Zero >> 8;
     AccelY_Zero = AccelY_Zero >> 8;
   //  AccelZ_Zero = AccelZ_Zero >> 8;
       
     Serial.print("Accel Zeros : ");
     
     Serial.print(AccelX_Zero);
     Serial.print(" , ");
     Serial.print(AccelY_Zero);
     Serial.println("");
       
}


void UpdateUserInput()
{
	int32_t i = 0;

    while(AccelRaw_Ready == 0)
     {
     }
            
     AccelRaw_Ready = 0;
    
    AccelX_Filtered = 0;
    AccelY_Filtered = 0;
    
    for(i=0;i<ACCEL_BUF_SIZE;i++)
    {
        AccelX_Filtered+= AccelBuff_X[i];
        AccelY_Filtered+= AccelBuff_Y[i];
    }
   
    AccelX_Filtered = AccelX_Filtered/ACCEL_BUF_SIZE;
    AccelY_Filtered = AccelY_Filtered/ACCEL_BUF_SIZE;
  

  	i = 32 + (PotRaw>>5); 

  	//Serial.println(i);
    GameBoard_X = (AccelX_Filtered-AccelX_Zero)/i;
    GameBoard_Y = (AccelY_Filtered-AccelY_Zero)/i;



   
 //  sprintf(StringBuf,"X: %i, Y: %i",GameBoard_X,GameBoard_Y);
	//Serial.println(StringBuf);
}



void adc0_timer_callback(void) 
{
    int i;
    
  ADC0_ReadPhase++;

  if(ADC0_ReadPhase > 2)
    ADC0_ReadPhase = 0;
    
  switch(ADC0_ReadPhase)
  {
    default:
    case 0:
      adc->startSingleRead(ACCEL_X_CHANNEL, ADC_0);
    break;

    case 1:
     adc->startSingleRead(ACCEL_Y_CHANNEL, ADC_0);
    break;
    
    case 2:
      adc->startSingleRead(POT_CHANNEL, ADC_0);
    break;
    
    
  }
  
 



}


void adc0_isr()
{



  switch(ADC0_ReadPhase)
  {
    default:
    case 0:
    AccelX_Raw = adc->readSingle()&0xFFFF;
    AccelBuff_X[AccelBuffIdx] = (int32_t)(AccelX_Raw);// - (int32_t)AccelX_Zero;
    
    break;

    case 1:
      AccelY_Raw = adc->readSingle()&0xFFFF;
    AccelBuff_Y[AccelBuffIdx] = (int32_t)(  AccelY_Raw );// - (int32_t)AccelY_Zero;
    
    AccelRaw_Ready = TRUE;
      
      AccelBuffIdx++;
      
      if(AccelBuffIdx >= ACCEL_BUF_SIZE)
        AccelBuffIdx = 0;
        
    break;

    case 3:
       PotRaw = adc->readSingle()&0xFFFF;
    break;
  }
}





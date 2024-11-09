#include "System.h"

volatile int32_t SystemTimers[NUM_SYSTEM_TIMERS];

char StringBuf[256];

IntervalTimer SystemTimer;

void SystemTimerCallBack()
{
	int i;
   	for(i=0;i<NUM_SYSTEM_TIMERS;i++)
	{
        if(SystemTimers[i] <0x7FFFFFFF)
            SystemTimers[i]++;
	}
}


void InitSystem()
{
	//Serial.println("Starting System Timers");
	SystemTimer.begin(SystemTimerCallBack,10000); //Every 10 milliseconds

}


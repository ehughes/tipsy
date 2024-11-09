#include "System.h"



void Init__Game__SensorTest()
{
	

}

void Process__Game__SensorTest()
{


Serial.println(SystemTimers[0]);
     UpdateUserInput();

     eGFX_ImagePlane_Clear(&eGFX_BackBuffer);
 
      eGFX_PutPixel(&eGFX_BackBuffer,14 + GameBoard_X,
                      14 -  GameBoard_Y, 0xFFFF00FF);
 


	eGFX_DrawHline(&eGFX_BackBuffer,
	                   0,
	                   eGFX_PHYSICAL_SCREEN_SIZE_X-1,
	                   0,
	                   0xFF00FF00);


	eGFX_DrawHline(&eGFX_BackBuffer,
	                   0,
	                   eGFX_PHYSICAL_SCREEN_SIZE_X-1,
	                   eGFX_PHYSICAL_SCREEN_SIZE_Y-1,
	                   0xFF00FF00);

	eGFX_DrawVline(&eGFX_BackBuffer,
	                   0,
	                   eGFX_PHYSICAL_SCREEN_SIZE_Y-1,
	                   eGFX_PHYSICAL_SCREEN_SIZE_X-1,
	                   0xFF00FF00);


	eGFX_DrawVline(&eGFX_BackBuffer,
	                   0,
	                   eGFX_PHYSICAL_SCREEN_SIZE_Y-1,
	                   0,
	                   0xFF00FF00);

/*
  	eGFX_printf(&eGFX_BackBuffer,
                      10,
                     1,
                     &FONT_3_5_1BPP,
                     
                     "%i,%i",GameBoard_X,GameBoard_Y);
*/
     eGFX_Dump(&eGFX_BackBuffer);
	

}

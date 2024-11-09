#include "eGFX_DataTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GFX_DRIVER_APA102_28_28
#define  GFX_DRIVER_APA102_28_28


#define eGFX_PHYSICAL_SCREEN_SIZE_X	((uint16_t) 28)	//This is the actual X and Y size of the physical screen in *pixels*
#define eGFX_PHYSICAL_SCREEN_SIZE_Y	((uint16_t) 28)


//These are the prototypes for the GFX HAL
extern void	eGFX_InitDriver();
extern void	eGFX_Dump(eGFX_ImagePlane *Image);

//A Driver *Must* have a backbuffer exposed
extern eGFX_ImagePlane eGFX_BackBuffer;



#endif


#ifdef __cplusplus
}
#endif
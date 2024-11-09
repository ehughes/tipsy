#include "System.h"

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

                          


PlayerInfo Player;


void ProcessStandardInput()
{
        Player.HistoryIndex++;
        if(Player.HistoryIndex>=PLAYER_HISTORY_SIZE)
            Player.HistoryIndex = 0;
        
        #ifdef _CONTROL_1
         Player.vx = (float)GameBoard_X  / VELOCITY_DIVIDER;
         Player.x += Player.vx;
         
         if(Player.x>eGFX_PHYSICAL_SCREEN_SIZE_X-1)
         {
            Player.x=eGFX_PHYSICAL_SCREEN_SIZE_X-1;
         Player.vx = 0;
         }  
         
         if(Player.x<0)
         {
            Player.x = 0;
            Player.vx = 0;
        }
           
         Player.vy = (float)GameBoard_Y  / VELOCITY_DIVIDER;
         Player.y -= Player.vy;
         
         if(Player.y>eGFX_PHYSICAL_SCREEN_SIZE_Y-1)
         {
            Player.y=eGFX_PHYSICAL_SCREEN_SIZE_Y-1;
              Player.vy = 0;
         }
            
         if(Player.y<0)
         {
            Player.y = 0;
            Player.vy = 0;
         }
         

        #endif
        
        #ifdef _CONTROL_2
        Player.x = (float)GameBoard_X+14;
        Player.y = 14 - (float)GameBoard_Y;
        
        #endif
            
        Player.X_History[Player.HistoryIndex] = Player.x;
        
        Player.Y_History[Player.HistoryIndex] = Player.y;
           
    }
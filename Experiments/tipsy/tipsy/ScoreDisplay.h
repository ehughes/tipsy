#ifndef SCORE_DISPLAY
#define SCORE_DISPLAY



#define GREEN_LEVEL      0xFF00FF00
#define YELLOW_LEVEL     0xFF00FFFF
#define RED_LEVEL        0xFF0000FF
#define BLUE_LEVEL       0xFFFF0000
#define PURPLE_LEVEL     0xFF8000FF
#define WHITE_LEVEL      0xFFA0A0A0


extern uint32_t MarqueePosition;

void ShowScoreDisplay(uint32_t Level, uint32_t Score, char *Msg);

void ProcessScoreDisplay();

#endif
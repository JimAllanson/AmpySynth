#include "AmpMenu.h"
#include "Peripherals.h"

#define GFX_WIDTH 240
#define GFX_HEIGHT 240
#define fontW 10
#define fontH 24

#define Black RGB565(0,0,0)
#define Red	RGB565(255,0,0)
#define Green RGB565(0,255,0)
#define Blue RGB565(0,0,255)
#define Gray RGB565(128,128,128)
#define LighterRed RGB565(255,150,150)
#define LighterGreen RGB565(150,255,150)
#define LighterBlue RGB565(150,150,255)
#define DarkerRed RGB565(150,0,0)
#define DarkerGreen RGB565(0,150,0)
#define DarkerBlue RGB565(0,0,150)
#define Cyan RGB565(0,255,255)
#define Magenta RGB565(255,0,255)
#define Yellow RGB565(255,255,0)
#define White RGB565(255,255,255)
#define AmpDarkBlue RGB565(26,34,45)
#define AmpPink RGB565(AMPLIENCE_PINK_R, AMPLIENCE_PINK_G, AMPLIENCE_PINK_B)

#define MAX_DEPTH 4


const colorDef<uint16_t> colors[6] MEMMODE={
{{(uint16_t)Black,(uint16_t)Black}, {(uint16_t)AmpDarkBlue, (uint16_t)AmpPink,  (uint16_t)AmpPink}},//bgColor
{{(uint16_t)Gray, (uint16_t)Gray},  {(uint16_t)White, (uint16_t)White, (uint16_t)White}},//fgColor
{{(uint16_t)White,(uint16_t)Black}, {(uint16_t)Yellow,(uint16_t)Yellow,(uint16_t)Red}},//valColor
{{(uint16_t)White,(uint16_t)Black}, {(uint16_t)White, (uint16_t)Yellow,(uint16_t)Yellow}},//unitColor
{{(uint16_t)White,(uint16_t)Gray},  {(uint16_t)AmpDarkBlue, (uint16_t)AmpPink,  (uint16_t)White}},//cursorColor
{{(uint16_t)White,(uint16_t)Yellow},{(uint16_t)AmpDarkBlue,  (uint16_t)AmpPink,   (uint16_t)White}},//titleColor
};

const panel panels[] MEMMODE = {{0, 0, GFX_WIDTH / fontW, GFX_HEIGHT / fontH}};
navNode* nodes[sizeof(panels) / sizeof(panel)];
panelsList pList(panels, nodes, 1);
idx_t eSpiTops[MAX_DEPTH]={0};
TFT_eSPIOut eSpiOut(tft,colors,eSpiTops,pList,fontW,fontH+1);
menuOut* constMEM outputs[] MEMMODE={&eSpiOut};//list of output devices
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller
serialIn serial(Serial);

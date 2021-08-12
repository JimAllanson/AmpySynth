#include "Arduino.h" 
#include "MainMenu.h" 
#include "Program.h" 
#include "AmpySynth.h" 
#include "Peripherals.h"
#include <menu.h>
#include <menuIO/serialIn.h>
#include <menuIO/serialOut.h>
#include <menuIO/TFT_eSPIOut.h>

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


int newProgram = 1;

void MainMenu::setup() {
    tft.fillScreen(TFT_BLACK);
/*
    int numPrograms = sizeof(programs)/sizeof(programs[0]);
    int menuSize = 0;
    for(int i = 1; i < numPrograms; i++) {
        if(programs[i].name != NULL) {
            menuSize++;
        }
    }
    prompt* menuData[menuSize];
    for(int i = 1; i < numPrograms; i++) {
        if(programs[i].name != NULL) {
            menuData[i] = new menuValue<int>(programs[i].name, i);
        }
    }*/


  /*  prompt* menuData[2] = {
    };

    constMEM promptShadows op1Info MEMMODE={(callback)doNothing,_noStyle,"apple",noEvent,noStyle, newProgram};
    constMEM promptShadows op2Info MEMMODE={(callback)doNothing,_noStyle,"banana",noEvent,noStyle, newProgram};
// constMEM promptShadow& op2Info=*(promptShadow*)&op2InfoRaw;
//or just this line on non MEMMODE devices like teensy or esp8266 instead of the above three
//promptShadow op2Info("Op 2",(callback)op2Func,enterEvent);
prompt op1(op1Info.obj);
prompt op2(op2Info.obj);
menuData[0] = &op1;
menuData[1] = &op2;


    constMEM menuNodeShadows menuInfo MEMMODE={
        (callback)doNothing,
        (systemStyles)(_menuData|_canNav),
        "AmpySynth",
        noEvent,
        noStyle,
        sizeof(menuData)/sizeof(prompt*),
        menuData
    };
    menuNode mainMenu(menuInfo.obj);
    //choose<int>& mainMenu =*new choose<int>("AmpySynth",newProgram,sizeof(menuData)/sizeof(prompt*),menuData);
    
*/
    CHOOSE(newProgram, mainMenu, "AmpySynth",doNothing,noEvent,wrapStyle
        ,VALUE("RGBAmpy",1,doNothing, noEvent)
        ,VALUE("BasicSynth",2,doNothing, noEvent)
    );

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
    NAVROOT(_nav,mainMenu,MAX_DEPTH,serial,out);

    nav = &_nav;

    nav->refresh();

    Serial.println("Finished menu setup");
}

void MainMenu::loop() {
    nav->poll();
}

void MainMenu::encoderUp() {
    nav->doNav(upCmd);
}

void MainMenu::encoderDown() {
    nav->doNav(downCmd);
}

void MainMenu::encoderPress() {
    exit(newProgram);
}
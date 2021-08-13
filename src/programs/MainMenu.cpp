#include "MainMenu.h"

int nextProgram = 1;

Menu::navNode navPath[4]; 
Menu::prompt* menuOpts[16];
Menu::choose<int> *mainMenu;
Menu::navRoot *nav;

void MainMenu::setup() {
    int numPrograms = sizeof(programs)/sizeof(programs[0]);
    int menuSize = 0;
    for(int i = 1; i < numPrograms; i++) {
        if(programs[i].name != NULL) {
            menuSize++;
        }
    }
    for(int i = 1; i <= menuSize; i++) {
        menuOpts[i - 1] = new menuValue<int>(programs[i].name, i);
    }
    mainMenu = new choose<int>("AmpySynth", nextProgram, menuSize, menuOpts);
    nav = new navRoot(*mainMenu,navPath,4,serial,out);
      
    nav->useMenu(*mainMenu);
    nav->doNav(downCmd);
    nav->refresh();
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
    exit(nextProgram);
}
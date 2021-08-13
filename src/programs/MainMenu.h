
#ifndef MAINMENU_H
#define MAINMENU_H

#include "AmpySynth.h"
#include "utils/Peripherals.h"
#include "utils/Program.h"
#include <utils/AmpMenu.h>
#include "Arduino.h" 
#include "menu.h"
#include <menuIO/serialIn.h>
#include <menuIO/serialOut.h>
#include <menuIO/TFT_eSPIOut.h>

class MainMenu : public Program {
    private:
        //Make Singleton
        MainMenu()  {};
        MainMenu(MainMenu const&);              // Don't Implement
        void operator=(MainMenu const&); // Don't implement
    public:
        static Program* getInstance() {
            static MainMenu instance;
            return &instance;
        }
        void setup();
        void loop();
        void encoderUp();
        void encoderDown();
        void encoderPress();
};

#endif
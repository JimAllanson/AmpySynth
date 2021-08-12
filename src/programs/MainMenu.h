
#ifndef MAINMENU_H
#define MAINMENU_H

#include "Program.h" 
#include <menu.h>

class MainMenu : public Program {
    private:
        //Make Singleton
        MainMenu(){};
        MainMenu(MainMenu const&);              // Don't Implement
        void operator=(MainMenu const&); // Don't implement
        navRoot *nav;
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
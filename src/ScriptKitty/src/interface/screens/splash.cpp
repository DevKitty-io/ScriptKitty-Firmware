#include "splash.h"
#include "../graphics.h"

SplashScreen::SplashScreen(unsigned long durationInMs) {
    this->endAt = millis()+durationInMs;
}

int SplashScreen::update(int button) {
    // update even when there's no keypress
    this->alwaysUpdates(true);
    unsigned int currentTime = millis();
    if (currentTime > this->endAt) {
        return SCREEN_BACK;
    }
    if (button==EVENT_INIT){
        return SCREEN_REDRAW;
    }
    return SCREEN_NONE;
}

bool SplashScreen::draw() {
  display->drawXbm(0, 0, 128, 64, ScriptKitty_Splash_bits);
//   display->drawString(104,0,"1.0");
//   display->drawRect(102,0,24,12);
  return true;
}

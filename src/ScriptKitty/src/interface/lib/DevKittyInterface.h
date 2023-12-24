#ifndef DEVKITTY_INTERFACE_H
#define DEVKITTY_INTERFACE_H

#include <Adafruit_NeoPixel.h>
#include "SH1106Wire.h"

//----------------------------------------
// DevKittyInputs

#define BTN_NONE   -1
#define BTN_PRESS   0
#define BTN_NPRESS  1

#define BTN_COUNT  4
#define BTN_UP     9
#define BTN_DOWN  18
#define BTN_LEFT  11
#define BTN_RIGHT  7

#define NEOPIXEL_PIN 12
#define NEOPIXEL_PIN_CNT 1

#define EVENT_INIT 100

class DevKittyInputs {
   public:
      DevKittyInputs();
      int getInput();
   private:
      void addButton(int);
      int buttons[BTN_COUNT];
      int pressedButton;
      int lastBtn;
};

//----------------------------------------
// DevKittyScreen

class DevKittyInterface;

#define SCREEN_NONE   1
#define SCREEN_BACK   2
#define SCREEN_REDRAW 3
#define SCREEN_PUSH   4

class DevKittyScreen {
   public:
      DevKittyScreen();
      virtual ~DevKittyScreen();
      virtual bool draw() = 0;
      virtual int update(int){return SCREEN_NONE;};
      void setDisplay(SH1106Wire*);
      void setInputs(DevKittyInputs*);
      void setStrip(Adafruit_NeoPixel*);
      void setDevKittyInterface(DevKittyInterface*);
      int _update();
   protected:
      SH1106Wire* display;
      DevKittyInputs* inputs;
      Adafruit_NeoPixel* strip;
      void pushScreen(DevKittyScreen*);
      void alwaysUpdates(bool);
   private:
      DevKittyInterface* DevKittyInterface;
      bool alwaysUpdate;
};

//----------------------------------------
// ScreenNode

struct ScreenNode {
   DevKittyScreen* screen;
   volatile ScreenNode* prev;
};


//----------------------------------------
// DevKittyInterface

class DevKittyInterface {
  public:
    DevKittyInterface();
    ~DevKittyInterface();
    bool start();
    bool draw();
    bool pushScreen(DevKittyScreen*);
    bool popScreen();
    bool injectScreen(DevKittyScreen*);
  private:
    SH1106Wire* display;
    DevKittyInputs* inputs;
    Adafruit_NeoPixel* strip;
    volatile ScreenNode* currentScreenNode;
    volatile bool* currentScreenHasRendered;
    volatile SemaphoreHandle_t screenLock;
};


#endif


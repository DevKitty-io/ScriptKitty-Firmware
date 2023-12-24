#include "DevKittyInterface.h"
#include "dejavu.h"

//----------------------------------------
// DevKittyInputs

DevKittyInputs::DevKittyInputs() {
  this->lastBtn = 0;
  this->pressedButton = -1;

  this->addButton(BTN_UP);
  this->addButton(BTN_DOWN);
  this->addButton(BTN_LEFT);
  this->addButton(BTN_RIGHT);
}

void DevKittyInputs::addButton(int pin) {
  pinMode(pin, INPUT_PULLUP);
  this->buttons[this->lastBtn++] = pin;
}

int DevKittyInputs::getInput() {
  int buttonState;
  if (this->pressedButton != BTN_NONE){
    buttonState = digitalRead(this->pressedButton);
    if (buttonState==BTN_PRESS){
      return BTN_NONE;
    }
    this->pressedButton = BTN_NONE;
    return BTN_NONE;
  }

  for (int i = 0; i < BTN_COUNT; i++){
    int btn = this->buttons[i];
    buttonState = digitalRead(btn);
    if (buttonState==BTN_PRESS){
      this->pressedButton = btn;
      return btn;
    }
  }

  this->pressedButton = -1;
  return BTN_NONE;
}

//----------------------------------------
// DevKittyScreen

DevKittyScreen::DevKittyScreen(){
  this->alwaysUpdate = false;
}
DevKittyScreen::~DevKittyScreen(){
}
void DevKittyScreen::setDisplay(SH1106Wire* display){
  this->display = display;
}
void DevKittyScreen::setInputs(DevKittyInputs* inputs){
  this->inputs = inputs;
}
void DevKittyScreen::setStrip(Adafruit_NeoPixel* strip){
  this->strip = strip;
}
void DevKittyScreen::setDevKittyInterface(DevKittyInterface* nI){
  this->devKittyInterface = nI;
}
void DevKittyScreen::pushScreen(DevKittyScreen* screen){
  this->devKittyInterface->pushScreen(screen);
}
void DevKittyScreen::alwaysUpdates(bool set){
  this->alwaysUpdate = set;
}
int DevKittyScreen::_update(){
  int btn = this->inputs->getInput();
  if (btn == BTN_NONE && !(this->alwaysUpdate)){
    delay(2);
    return SCREEN_NONE;
  }
  return this->update(btn);
}

//----------------------------------------
// DevKittyInterface
DevKittyInterface::DevKittyInterface(){
  SH1106Wire* nDisplay = new SH1106Wire(0x3C, 33, 35);
  this->inputs = new DevKittyInputs();
  this->screenLock = xSemaphoreCreateMutex();
  if (this->screenLock == nullptr) {
    Serial.println("[DevKittyInterface] mutex could not be created");
  }
  nDisplay->init();
  nDisplay->flipScreenVertically();
  nDisplay->setTextAlignment(TEXT_ALIGN_LEFT);
  nDisplay->setFont(DejaVu_Sans_Mono_10);
  this->display = nDisplay;
  this->currentScreenNode = nullptr;

  pinMode(NEOPIXEL_PIN, OUTPUT);
  this->strip = new Adafruit_NeoPixel(NEOPIXEL_PIN_CNT, NEOPIXEL_PIN, NEO_RGB + NEO_KHZ800);
  this->strip->begin();
}

DevKittyInterface::~DevKittyInterface(){
  while (this->currentScreenNode != nullptr){
    this->popScreen();
  }
  delete this->display;
  delete this->inputs;
  delete this->strip;
}

bool DevKittyInterface::start(){
  while (true) {
    if (xSemaphoreTake(this->screenLock, 0)==pdFALSE) {
      delay(10);
      continue;
    }
    int action = SCREEN_NONE;
    if (!currentScreenHasRendered) {
      this->draw();
      action = this->currentScreenNode->screen->update(EVENT_INIT);
      this->currentScreenHasRendered = (volatile bool*) true;
    } else {
      action = this->currentScreenNode->screen->_update();
    }

    if (action==SCREEN_BACK){
      this->popScreen();
    }
    if (action==SCREEN_REDRAW){
      this->draw();
    }
    if (action==SCREEN_PUSH){
      this->draw();
      this->currentScreenNode->screen->update(EVENT_INIT);
    }
    xSemaphoreGive(this->screenLock);
  }
}

// injectScreen pushes a screen onto the stack e.g. from a thread that did
// not call DevKittyInterface::start. This function returns false if the screen
// could not be locked in wait TICKS
bool DevKittyInterface::injectScreen(DevKittyScreen* screen){
  if (xSemaphoreTake(this->screenLock, 200)==pdFALSE) {
    // could not acquire lock
    return false;
  }
  this->pushScreen(screen);
  //this->draw();
  //this->currentScreenNode->screen->update(EVENT_INIT);
  xSemaphoreGive(this->screenLock);
}

bool DevKittyInterface::pushScreen(DevKittyScreen* screen){
  if (screen==nullptr) {
      return false;
  }
  screen->setDisplay(this->display);
  screen->setInputs(this->inputs);
  screen->setStrip(this->strip);
  screen->setDevKittyInterface(this);
  ScreenNode* nextScreenNode = new ScreenNode;
  nextScreenNode->prev = this->currentScreenNode;
  nextScreenNode->screen = screen;

  this->currentScreenNode = nextScreenNode;
  this->currentScreenHasRendered = (volatile bool*) false;
  return true;
}

bool DevKittyInterface::popScreen(){
  if (this->currentScreenNode->prev == nullptr){
    return false;
  }
  volatile ScreenNode* popped = this->currentScreenNode;
  this->currentScreenNode = this->currentScreenNode->prev;
  this->currentScreenHasRendered = (volatile bool*) false;
  delete popped->screen;
  delete popped;

  return true;
}

bool DevKittyInterface::draw() {
  if (this->currentScreenNode == nullptr){
    return false;
  }
  this->display->clear();
  this->currentScreenNode->screen->draw();
  this->display->display();
  return true;
}

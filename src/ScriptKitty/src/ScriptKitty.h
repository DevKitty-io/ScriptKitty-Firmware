#pragma once

#include "Arduino.h"
#include "cdcusb.h"
#include "mscusb.h"
#include "flashdisk.h"

class ScriptKitty {
  public:
    ScriptKitty(){};
    static void init();
    static String* allPayloadPaths(const char* path="/");
};

struct ScriptKittyConfig {
  String locale;
  String network;
  String password;
  long pid;
  long vid;
};

FILINFO* newFileList(const char* path, int& numFiles);
ScriptKittyConfig getConfig();

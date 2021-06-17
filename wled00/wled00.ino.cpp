# 1 "C:\\Users\\pablo\\AppData\\Local\\Temp\\tmp47i72mvi"
#include <Arduino.h>
# 1 "D:/OneDrive - UNIVERSIDAD ALICANTE/Personal/Microcontroladores/Wled/WLED-0.12.0/WLED/wled00/wled00.ino"
# 13 "D:/OneDrive - UNIVERSIDAD ALICANTE/Personal/Microcontroladores/Wled/WLED-0.12.0/WLED/wled00/wled00.ino"
#include "wled.h"
void setup();
void loop();
#line 15 "D:/OneDrive - UNIVERSIDAD ALICANTE/Personal/Microcontroladores/Wled/WLED-0.12.0/WLED/wled00/wled00.ino"
void setup() {
  WLED::instance().setup();
}

void loop() {
  WLED::instance().loop();
}
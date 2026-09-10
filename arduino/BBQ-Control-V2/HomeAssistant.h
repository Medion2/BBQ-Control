#pragma once
#include <Arduino.h>
void haBegin();
void haUpdate();
bool haLive();
String haStatusText();
const struct ProbeData &haProbe(unsigned index);
unsigned haSelected();
void haSelect(unsigned index);
String haProbeName();

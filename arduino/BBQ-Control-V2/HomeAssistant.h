#pragma once
#include <Arduino.h>
void haBegin();
void haUpdate();
bool haLive();
// Last successful response stays usable within StaleMs; errors remain visible.
bool haDataAvailable();
bool haRecovering();
String haStatusText();
const struct ProbeData &haProbe(unsigned index);
unsigned haSelected();
void haSelect(unsigned index);
String haProbeName();

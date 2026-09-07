#pragma once
#include <Arduino.h>
#include <time.h>
void clockUpdate();
bool clockLocalTime(tm &local);
String clockStatusText();

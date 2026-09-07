#pragma once
#include <Arduino.h>
void mqttBegin();
void mqttUpdate();
bool mqttLive();
String mqttStatusText();
String haStatusText();

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <math.h>
struct ProbeData {
  float core = NAN, ambient = NAN, target = NAN, battery = NAN;
  float peak=NAN, remaining=NAN, elapsed=NAN;
  char cooking[64]="";
  char cookState[32] = "";
  bool probeOnline = false;
  bool received = false;
  uint32_t receivedAt = 0;
};
const ProbeData &probeData();
bool probeFresh(uint32_t now);
// Main-loop access only; MQTT callbacks must hand data through a queue.
void probeSet(const ProbeData &value);
bool probeParse(const char *json, size_t length, uint32_t now, ProbeData &value);

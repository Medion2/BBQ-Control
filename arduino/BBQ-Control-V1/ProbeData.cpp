#include "ProbeData.h"
#include "Config.h"
namespace { ProbeData data; }
const ProbeData &probeData() { return data; }
bool probeFresh(uint32_t now) {
  return data.received && data.probeOnline && uint32_t(now-data.receivedAt) < Config::StaleMs;
}
void probeSet(const ProbeData &value) { data = value; }

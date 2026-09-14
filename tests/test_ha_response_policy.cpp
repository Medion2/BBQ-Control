#include "../arduino/BBQ-Control-V2/HaResponsePolicy.h"
using HaResponsePolicy::usable;
static_assert(!usable(false,true,-1,1000,0,90000),"Startup failure has no cached data");
static_assert(usable(true,true,200,1000,1000,90000),"New successful response");
static_assert(usable(true,true,-11,20000,1000,90000),"Read timeout retains dated values");
static_assert(usable(true,true,503,90999,1000,90000),"Transient outage within TTL");
static_assert(!usable(true,true,503,91000,1000,90000),"Expiry is exact, even with repeated failures");
static_assert(!usable(true,true,200,91000,1000,90000),"A stalled worker cannot keep data fresh");
static_assert(!usable(true,false,200,2000,1000,90000),"WiFi loss invalidates live readings");
static_assert(!usable(true,true,401,2000,1000,90000),"Rejected token invalidates cached data");
static_assert(!usable(true,true,403,2000,1000,90000),"Forbidden invalidates cached data");
static_assert(usable(true,true,200,100000,100000,90000),"Recovery starts a new validity window");
static_assert(usable(true,true,-11,0x20u,0xfffffff0u,90000),"millis wraparound retains recent data");
static_assert(!usable(true,true,-11,0x00015f80u,0xfffffff0u,90000),"millis wraparound still expires data");

using HaResponsePolicy::retryConnection;
static_assert(retryConnection(-1,0,true),"TCP setup failure gets one retry");
static_assert(!retryConnection(-1,1,true),"Second failure stops immediate retries");
static_assert(!retryConnection(-1,0,false),"No retry while WiFi is down");
static_assert(!retryConnection(401,0,true),"Authentication errors are not TCP retries");
static_assert(!retryConnection(503,0,true),"Server responses use normal polling cadence");

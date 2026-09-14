#pragma once
#include <stdint.h>
namespace HaResponsePolicy {
// Errors cannot renew receivedAt. Authentication failures invalidate cached data immediately.
constexpr bool usable(bool received,bool wifi,int code,uint32_t now,uint32_t receivedAt,uint32_t ttl){
 return received && wifi && code!=401 && code!=403 && uint32_t(now-receivedAt)<ttl;
}
// HTTPClient -1 means TCP setup failed, not an HTTP status returned by HA.
constexpr bool retryConnection(int code,unsigned attempt,bool wifi){return code==-1 && attempt==0 && wifi;}
}

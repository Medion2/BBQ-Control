#include "ProbeData.h"
#include "Config.h"
namespace { ProbeData data; }
const ProbeData &probeData() { return data; }
bool probeFresh(uint32_t now) {
  return data.received && data.probeOnline && uint32_t(now-data.receivedAt) < Config::StaleMs;
}
void probeSet(const ProbeData &value) { data = value; }

#include "cJSON.h"
#include <string.h>
namespace {
bool numberOrNull(const cJSON *root, const char *key, float min, float max, float &value) {
  const cJSON *item=cJSON_GetObjectItemCaseSensitive(root,key);
  if(cJSON_IsNull(item)) { value=NAN; return true; }
  if(!cJSON_IsNumber(item) || !isfinite(item->valuedouble) || item->valuedouble<min || item->valuedouble>max) return false;
  value=float(item->valuedouble); return true;
}
}
bool probeParse(const char *json, size_t length, uint32_t now, ProbeData &value) {
  if(!json || length==0 || length>Config::MaxPayload || memchr(json,'\0',length)) return false;
  // Input from MQTT is NUL-terminated at length. Reject trailing garbage.
  cJSON *root=cJSON_ParseWithLengthOpts(json,length+1,nullptr,true);
  if(!root) return false;
  ProbeData next;
  const cJSON *schema=cJSON_GetObjectItemCaseSensitive(root,"schema");
  const cJSON *online=cJSON_GetObjectItemCaseSensitive(root,"probe_online");
  const cJSON *state=cJSON_GetObjectItemCaseSensitive(root,"cook_state");
  bool valid=cJSON_IsObject(root) && cJSON_IsNumber(schema) && schema->valuedouble==1 &&
    cJSON_IsBool(online) && (cJSON_IsNull(state) || cJSON_IsString(state));
  if(valid) valid=numberOrNull(root,"core_c",-40,150,next.core) &&
    numberOrNull(root,"ambient_c",-40,600,next.ambient) &&
    numberOrNull(root,"target_c",0,150,next.target) &&
    numberOrNull(root,"battery_pct",0,100,next.battery);
  if(valid && cJSON_IsString(state)) {
    size_t n=strlen(state->valuestring);
    if(n>=sizeof(next.cookState)) valid=false;
    else {
      // The built-in GFX font is ASCII; reject controls and replace unsupported bytes.
      for(size_t i=0;i<n;++i) {
        unsigned char ch=state->valuestring[i];
        if(ch<32 || ch==127) { valid=false; break; }
        next.cookState[i]=ch<127?char(ch):'?';
      }
    }
  }
  if(valid) {
    next.probeOnline=cJSON_IsTrue(online); next.received=true; next.receivedAt=now;
    value=next;
  }
  cJSON_Delete(root); return valid;
}

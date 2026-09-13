from pathlib import Path
import math
pixels=[]
for y in range(249):
 for x in range(249):
  dx=x-124;dy=y-124;r=math.hypot(dx,dy);a=(math.degrees(math.atan2(dy,dx))-135)%360
  alpha=min(1,4.5-abs(r-118))
  if a<=270 and alpha>0:pixels.append(x|(y<<8)|(round(a/270*255)<<16)|(round(alpha*255)<<24))
out=Path(__file__).resolve().parents[1]/'arduino/BBQ-Control-V2/ReferenceRing.h'
out.write_text('#pragma once\n#include <stdint.h>\nstatic const uint32_t ReferenceRing[]={\n'+','.join('0x%08xu'%v for v in pixels)+'\n};\n',encoding='utf-8')
print(len(pixels),'antialiased ring pixels generated')

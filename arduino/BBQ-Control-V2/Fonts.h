#pragma once
#include <stdint.h>
struct Glyph { uint32_t offset; int16_t x,y; uint8_t w,h,advance; };
struct SmoothFont { const Glyph *glyphs; const uint8_t *alpha; uint8_t ascent,height; };
extern const SmoothFont SmallFont, LabelFont, ValueFont, HeroFont;

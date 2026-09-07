#pragma once
#include <Arduino.h>
namespace Config {
constexpr uint16_t Width = 480, Height = 480;
constexpr uint32_t FrameMs = 33, BootMs = 1800;
constexpr uint8_t TouchAddress = 0x5D;
constexpr uint16_t Background = 0x0841, White = 0xFFFF, Accent = 0x07FF;
}

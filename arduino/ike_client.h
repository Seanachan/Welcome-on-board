#pragma once
#include <Arduino.h>
#include <BLEDevice.h>

// Initialize BLE client for I-KE-V3.
// Call this once in setup(), *after* BLEDevice::init().
void ikeInit();

// Call this every loop() so scanning / reconnect can work.
void ikeLoop();

// High-level APIs like your Python functions:
bool ikeSetStatic(int r, int g, int b, int brightness = 0xFF);
bool ikeSetBreathe(int r, int g, int b, int speed = 0x03);
bool ikeSetBlink(int r, int g, int b, int speed = 0x03);

// Optional: send raw 20-byte packet from 40-char hex string
bool ikeSendHexPacket(const String &hex);

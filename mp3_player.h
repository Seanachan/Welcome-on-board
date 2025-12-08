#pragma once

#include <Arduino.h>

// Initialize DFPlayer + UART
void mp3Init();

// Play a specific track (1 = 0001.mp3, 2 = 0002.mp3, ...)
void mp3PlayTrack(uint16_t track);

// Optional helpers (for future use)
void mp3Stop();
void mp3SetVolume(uint8_t vol); // 0~30

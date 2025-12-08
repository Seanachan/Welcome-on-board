#include "mp3_player.h"
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// We’ll use UART1 for DFPlayer
// Wiring (你現在已經測過 OK 的那組):
// DFPlayer TX -> ESP32 IO19
// ESP32 IO23 -> (1k resistor) -> DFPlayer RX
// DFPlayer VCC -> 5V, GND -> GND
HardwareSerial mp3Serial(1);
static DFRobotDFPlayerMini mp3;
static bool mp3Ready = false;

void mp3Init()
{
  Serial.println("[MP3] Init...");

  // RX=19, TX=23
  mp3Serial.begin(9600, SERIAL_8N1, 19, 23);
  delay(1000);

  if (!mp3.begin(mp3Serial))
  {
    Serial.println("[MP3] DFPlayer init FAILED (check wiring/SD).");
    mp3Ready = false;
    return;
  }

  mp3Ready = true;
  Serial.println("[MP3] DFPlayer online.");

  // Default volume
  mp3.volume(25); // 0~30
  Serial.println("[MP3] Volume set to 25.");

  // Just for debug:
  int files = mp3.readFileCounts();
  Serial.print("[MP3] Total files on SD = ");
  Serial.println(files);
}

void mp3PlayTrack(uint16_t track)
{
  if (!mp3Ready)
  {
    Serial.println("[MP3] Not ready, call mp3Init() first or check wiring.");
    return;
  }
  Serial.print("[MP3] Playing track ");
  Serial.println(track);
  mp3.play(track);
}

void mp3Stop()
{
  if (!mp3Ready)
    return;
  Serial.println("[MP3] Stop.");
  mp3.stop();
}

void mp3SetVolume(uint8_t vol)
{
  if (!mp3Ready)
    return;
  if (vol > 30)
    vol = 30;
  Serial.print("[MP3] Set volume to ");
  Serial.println(vol);
  mp3.volume(vol);
}

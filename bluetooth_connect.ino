#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_system.h>

#include "ike_client.h"
#include "mp3_player.h"
// --- WROVER SAFE PINS ---
#define RX_PIN 26
#define TX_PIN 27
#define BAUDRATE 9600

// --- BLE UUIDs (SYNCHRONIZED WITH REACT SCRIPT) ---
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
typedef struct LED_Color
{
  int r;
  int g;
  int b;
  int speed;
  char mode; // 0=static,1=breathe,2=blink
} LED_Color;

HardwareSerial SerialPIC(2);
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool mp3SyncPlaying = false;
unsigned long mp3StartMillis = 0;
int mp3BeatIndex = 0;
unsigned char volume = 25; // default volume
String usbLine;
const int NUM_BEATS = 12;
const int BEAT_OFFSET_MS = 120;
const unsigned int beatTimesMs[NUM_BEATS] = {
    827,   // cue 0
    1675,  // cue 1
    2523,  // cue 2
    2951,  // cue 3
    3378,  // cue 4
    4226,  // cue 5
    4226,  // cue 6
    12846, // cue 23
    16287, // cue 24
    17136, // cue 25
    17987, // cue 26
    19266  // cue 27
};

LED_Color beatColors[NUM_BEATS] = {
    {255, 0, 0, 3, 0},    // cue 0
    {255, 140, 58, 3, 0}, // cue 1
    {255, 255, 0, 3, 0},  // cue 2
    {0, 255, 0, 3, 0},    // cue 3
    {0, 255, 255, 3, 0},  // cue 4
    {0, 0, 255, 3, 0},    // cue 5
    {75, 0, 130, 1, 2},   // cue 6
    {138, 43, 226, 1, 1}, // cue 23
    {199, 21, 133, 1, 1}, // cue 24
    {255, 20, 147, 1, 2}, // cue 25
    {255, 69, 0, 1, 2},   // cue 26
    {255, 140, 58, 1, 2}  // cue 27
};
enum VoiceAction
{
  VOICE_NONE = 0,
  VOICE_LIGHT_RED,
  VOICE_LIGHT_BLUE,
  VOICE_LIGHT_GREEN,
  VOICE_PLAY_MUSIC,
  VOICE_STOP_MUSIC,
  VOICE_VOL_UP,
  VOICE_VOL_DOWN
};

volatile VoiceAction g_pendingVoiceAction = VOICE_NONE;

void handleUSBCommand(const String &line)
{
  String cmd = line;
  cmd.trim();
  if (cmd.length() == 0)
    return;

  String lower = cmd;
  lower.toLowerCase();

  Serial.print("[USB CMD] ");
  Serial.println(cmd);
  if (lower == "play")
  {
    // play 0001.mp3 (or 0002.mp3 if you needed)
    mp3PlayTrack(1);

    // sync light stick effect as you like:
    // ikeSetBreathe(255, 140, 58, 3); // speed=3

    mp3SyncPlaying = true;
    mp3StartMillis = millis();
    mp3BeatIndex = 0;
    Serial.println("[SYNC] Started beat-synced playback.");
    return;
  }
  else if (lower == "off")
  {
    ikeSetStatic(0, 0, 0, 0x00);
    return;
  }

  else if (lower.startsWith("static"))
  {
    int r, g, b, bright = 0xFF;
    int n = sscanf(cmd.c_str(), "%*s %d %d %d %d", &r, &g, &b, &bright);
    if (n < 3)
    {
      Serial.println("Usage: static R G B [brightness]");
      return;
    }
    ikeSetStatic(r, g, b, bright);
    return;
  }

  else if (lower.startsWith("breathe"))
  {
    int r, g, b, speed = 0x03;
    int n = sscanf(cmd.c_str(), "%*s %d %d %d %d", &r, &g, &b, &speed);
    if (n < 3)
    {
      Serial.println("Usage: breathe R G B [speed]");
      return;
    }
    ikeSetBreathe(r, g, b, speed);
    return;
  }

  else if (lower.startsWith("blink"))
  {
    int r, g, b, speed = 0x03;
    int n = sscanf(cmd.c_str(), "%*s %d %d %d %d", &r, &g, &b, &speed);
    if (n < 3)
    {
      Serial.println("Usage: blink R G B [speed]");
      return;
    }
    ikeSetBlink(r, g, b, speed);
    return;
  }

  if (lower.startsWith("packet"))
  {
    int spaceIdx = cmd.indexOf(' ');
    if (spaceIdx < 0 || spaceIdx + 1 >= cmd.length())
    {
      Serial.println("Usage: packet HEXSTRING");
      return;
    }
    String hex = cmd.substring(spaceIdx + 1);
    hex.trim();
    hex.replace(" ", "");
    hex.replace("-", "");
    hex.replace("0x", "");
    if (!ikeSendHexPacket(hex))
    {
      Serial.println("Invalid hex string or wrong length (need 40 hex chars).");
    }
    return;
  }

  Serial.println("Unknown command.");
}

// --- 1. CALLBACKS FOR CONNECTION EVENTS ---
void handleBeatCue(int cueIndex)
{
  LED_Color lc = beatColors[cueIndex];
  switch (lc.mode)
  {
  case 0: // static
    ikeSetStatic(lc.r, lc.g, lc.b, 0xFF);
    break;
  case 1: // breathe
    ikeSetBreathe(lc.r, lc.g, lc.b, lc.speed);
    break;
  case 2: // blink
    ikeSetBlink(lc.r, lc.g, lc.b, lc.speed);
    break;
  default:
    break;
  }
}
// Return true if the command was handled locally (ESP32), false if it should be forwarded to PIC.
bool handleVoiceCommand(const String &cmdRaw)
{
  String cmd = cmdRaw;
  cmd.trim();
  cmd.toUpperCase();

  if (cmd.length() == 0)
    return false;

  Serial.print("[VOICE CMD] ");
  Serial.println(cmd);
  // --------- MUSIC (handled on ESP32) ---------
  if (cmd == "PLAY_MUSIC")
  {
    g_pendingVoiceAction = VOICE_PLAY_MUSIC;
    return true;
  }

  else if (cmd == "STOP_MUSIC")
  {
    g_pendingVoiceAction = VOICE_STOP_MUSIC;
    return true;
  }

  else if (cmd == "VOL_UP")
  {
    g_pendingVoiceAction = VOICE_VOL_UP;
    return true;
  }

  else if (cmd == "VOL_DOWN")
  {
    g_pendingVoiceAction = VOICE_VOL_DOWN;
    return true;
  }

  // --------- LIGHTS (handled on ESP32, via I-KE) ---------

  else if (cmd == "RED")
  {
    g_pendingVoiceAction = VOICE_LIGHT_RED;
    return true;
  }

  else if (cmd == "BLUE")
  {
    g_pendingVoiceAction = VOICE_LIGHT_BLUE;
    return true;
  }

  else if (cmd == "GREEN")
  {
    g_pendingVoiceAction = VOICE_LIGHT_GREEN;
    return true;
  }

  // movement / steering / gears / HONK → let PIC handle it
  return false;
}

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    Serial.println("--- BLE Connection Successful! ---");

    const char *connectMsg = "CONNECTED";
    pTxCharacteristic->setValue(connectMsg);
    pTxCharacteristic->notify();
    Serial.println("Sent 'CONNECTED' notification to Web App.");
  };

  void onDisconnect(BLEServer *pServer) override
  {
    deviceConnected = false;
    oldDeviceConnected = false;
    Serial.println("Device disconnected. Restarting advertising...");

    // Immediately restart advertising so phone/web can reconnect
    pServer->getAdvertising()->start();
    Serial.println("Advertising started again.");
  }
};

// --- 2. CALLBACKS FOR RECEIVING DATA FROM WEB APP ---
class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    uint8_t *data = pCharacteristic->getData();
    size_t len = pCharacteristic->getLength();
    if (len == 0)
      return;

    // Example: interpret ASCII command starting with "IKE "
    // e.g. "IKE STATIC 255 0 0 255"
    String cmd = "";
    for (size_t i = 0; i < len; ++i)
      cmd += (char)data[i];

    if (handleVoiceCommand(cmd))
    {
      return; // already handled locally
    }

    // Default behavior: forward everything to PIC
    for (size_t i = 0; i < len; i++)
    {
      SerialPIC.write(data[i]);
      Serial.print((char)data[i]); // Debug
    }
  }
};

// --- SETUP ---
void setup()
{
  Serial.begin(9600);

  uint32_t seed = esp_random();
  randomSeed(seed);

  mp3Init(); // Initialize MP3 player

  // UART to PIC
  SerialPIC.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);

  // Common BLE init (shared by server + client)
  BLEDevice::init("ESP32_Voice_Bridge");

  // ---- BLE Server (Web app bridge) ----
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);
  pTxCharacteristic->addDescriptor(new BLE2902());
  pTxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();

  Serial.println("BLE Started! UUIDs synchronized with React App.");

  // ---- I-KE-V3 BLE Client ----
  ikeInit(); // start in "scan mode" for I-KE-V3
}

// --- MAIN LOOP ---
void loop()
{
  // 0. USB Serial -> command line
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == '\r')
      continue;
    if (c == '\n')
    {
      handleUSBCommand(usbLine); // process full line
      usbLine = "";
    }
    else
    {
      usbLine += c; // build up the line
    }
  }

  if (mp3SyncPlaying && mp3BeatIndex < NUM_BEATS)
  {
    unsigned long elapsed = millis() - mp3StartMillis;
    uint16_t target = beatTimesMs[mp3BeatIndex] + BEAT_OFFSET_MS;

    if (elapsed >= target)
    {
      handleBeatCue(mp3BeatIndex); // will be called 3 times total
      mp3BeatIndex++;
      if (mp3BeatIndex >= NUM_BEATS)
      {
        mp3SyncPlaying = false;
        ikeSetStatic(120, 120, 120, 0xFF);
      }
    }
  }
  // --- Process pending voice actions (run in main loop, not in BLE callback) ---
  if (g_pendingVoiceAction != VOICE_NONE)
  {
    VoiceAction action = g_pendingVoiceAction;
    g_pendingVoiceAction = VOICE_NONE; // clear first, in case action takes time

    Serial.print("[VOICE EXEC] action = ");
    Serial.println((int)action);

    switch (action)
    {
    case VOICE_PLAY_MUSIC:
      mp3PlayTrack(1); // or whichever track you want
      break;

    case VOICE_STOP_MUSIC:
      mp3Stop();
      break;

    case VOICE_VOL_UP:
      mp3SetVolume(25); // adjust according to your global volume logic
      break;

    case VOICE_VOL_DOWN:
      mp3SetVolume(10);
      break;

    case VOICE_LIGHT_RED:
      ikeSetStatic(255, 0, 0, 0xFF);
      break;

    case VOICE_LIGHT_BLUE:
      ikeSetStatic(0, 0, 255, 0xFF);
      break;

    case VOICE_LIGHT_GREEN:
      ikeSetStatic(0, 255, 0, 0xFF);
      break;

    default:
      break;
    }
  }
  // Maintain I-KE-V3 connection & handle scan/reconnect
  ikeLoop();

  // --- PIC -> WEB APP (Transmit) ---
  if (deviceConnected && SerialPIC.available())
  {
    String str = "";
    while (SerialPIC.available())
    {
      str += (char)SerialPIC.read();
      delay(2);
    }
    if (str.length() > 0)
    {
      pTxCharacteristic->setValue((uint8_t *)str.c_str(), str.length());
      pTxCharacteristic->notify();
      Serial.print("Sent to App: ");
      Serial.println(str);
    }
  }

  // --- CONNECTION MANAGEMENT ---
  if (deviceConnected && !oldDeviceConnected)
  {
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected to Web App!");
  }

  delay(1);
}
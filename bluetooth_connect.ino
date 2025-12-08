#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "ike_client.h"

// --- WROVER SAFE PINS ---
#define RX_PIN 26
#define TX_PIN 27
#define BAUDRATE 9600

// --- BLE UUIDs (SYNCHRONIZED WITH REACT SCRIPT) ---
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

HardwareSerial SerialPIC(2);
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

String usbLine;
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

  if (lower == "off")
  {
    ikeSetStatic(0, 0, 0, 0x00);
    return;
  }

  if (lower.startsWith("static"))
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

  if (lower.startsWith("breathe"))
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

  if (lower.startsWith("blink"))
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

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    Serial.println("Device disconnected. Starting advertising...");
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

    // Simple command routing (optional – you can delete this if not needed yet)
    if (cmd.startsWith("IKE "))
    {
      Serial.print("[CMD] ");
      Serial.println(cmd);
      // Very minimal parsing demo:
      // IKE STATIC r g b brightness
      if (cmd.startsWith("IKE STATIC"))
      {
        int r, g, b, br;
        if (sscanf(cmd.c_str(), "IKE STATIC %d %d %d %d", &r, &g, &b, &br) == 4)
        {
          ikeSetStatic(r, g, b, br);
        }
      }
      else if (cmd.startsWith("IKE BREATHE"))
      {
        int r, g, b, sp;
        if (sscanf(cmd.c_str(), "IKE BREATHE %d %d %d %d", &r, &g, &b, &sp) == 4)
        {
          ikeSetBreathe(r, g, b, sp);
        }
      }
      else if (cmd.startsWith("IKE BLINK"))
      {
        int r, g, b, sp;
        if (sscanf(cmd.c_str(), "IKE BLINK %d %d %d %d", &r, &g, &b, &sp) == 4)
        {
          ikeSetBlink(r, g, b, sp);
        }
      }
      // You can add "IKE HEX <40-hex>" => ikeSendHexPacket(...)
      return; // Already handled, don't forward to PIC
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
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);
    pServer->startAdvertising();
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected)
  {
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected to Web App!");
  }

  delay(1);
}
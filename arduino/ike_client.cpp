#include "ike_client.h"

// ====== CONFIG ======
static const char *IKE_DEVICE_NAME = "I-KE-V3";

// Characteristic UUID you already know
static BLEUUID IKE_CHAR_LED_UUID("8EC91004-F315-4F60-9FB8-838830DAEA50");

// light stick modes
static const uint8_t MODE_STATIC = 0x20;
static const uint8_t MODE_BREATHE = 0x21;
static const uint8_t MODE_BLINK = 0x11;

// ====== INTERNAL STATE ======
static BLEClient *pIKEClient = nullptr;
static BLERemoteCharacteristic *pIKECharLED = nullptr;
static BLEAdvertisedDevice *pIKEAdvertised = nullptr;
static bool ikeConnected = false;
static bool ikeDoConnect = false;
static bool ikeDoScan = false;

// ====== INTERNAL HELPERS ======
static uint8_t clamp_u8(int x)
{
  if (x < 0)
    return 0;
  if (x > 255)
    return 255;
  return (uint8_t)x;
}

static void buildPacket(uint8_t mode_byte,
                        int r, int g, int b,
                        int extra,
                        uint8_t tail_byte,
                        uint8_t out[20])
{
  uint8_t R = clamp_u8(r);
  uint8_t G = clamp_u8(g);
  uint8_t B = clamp_u8(b);
  uint8_t E = clamp_u8(extra);

  out[0] = mode_byte;
  out[1] = 0x00;
  out[2] = 0x00;
  out[3] = 0x00;
  out[4] = 0x00;
  out[5] = 0x00;
  out[6] = 0x00;
  out[7] = 0x00;
  out[8] = 0x00;
  out[9] = 0x00;
  out[10] = R;
  out[11] = G;
  out[12] = B;
  out[13] = E;
  out[14] = 0x00;
  out[15] = 0x00;
  out[16] = 0x00;
  out[17] = 0x00;
  out[18] = 0x01;
  out[19] = tail_byte;
}

static bool sendPacketInternal(const uint8_t pkt[20])
{
  if (!ikeConnected || pIKECharLED == nullptr)
  {
    Serial.println("[IKE] Not connected or LED characteristic missing.");
    return false;
  }

  Serial.print("[IKE] Sending: ");
  for (int i = 0; i < 20; ++i)
  {
    if (pkt[i] < 16)
      Serial.print('0');
    Serial.print(pkt[i], HEX);
  }
  Serial.println();

  // write with response
  pIKECharLED->writeValue((uint8_t *)pkt, 20, true);

  // read response
  String resp = pIKECharLED->readValue();
  Serial.print("[IKE] Response: ");
  for (size_t i = 0; i < resp.length(); ++i)
  {
    uint8_t v = (uint8_t)resp[i];
    if (v < 16)
      Serial.print('0');
    Serial.print(v, HEX);
  }
  Serial.println();

  return true;
}

// ====== BLE CLIENT CALLBACKS ======
class IKEClientCallbacks : public BLEClientCallbacks
{
  void onConnect(BLEClient *client) override
  {
    Serial.println("[IKE] Client connected.");
  }
  void onDisconnect(BLEClient *client) override
  {
    Serial.println("[IKE] Client disconnected.");
    ikeConnected = false;
  }
};

class IKEAdvertisedCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice) override
  {
    String name = advertisedDevice.getName().c_str();
    Serial.print("[Scan] Found: ");
    Serial.println(name);

    if (name.indexOf(IKE_DEVICE_NAME) >= 0)
    {
      Serial.println("[Scan] I-KE-V3 found.");
      BLEDevice::getScan()->stop();
      pIKEAdvertised = new BLEAdvertisedDevice(advertisedDevice);
      ikeDoConnect = true;
      ikeDoScan = false;
    }
  }
};

static bool connectToIKE()
{
  if (pIKEAdvertised == nullptr)
    return false;

  Serial.println("[IKE] Connecting to I-KE-V3...");
  pIKEClient = BLEDevice::createClient();
  pIKEClient->setClientCallbacks(new IKEClientCallbacks());
  pIKEClient->connect(pIKEAdvertised);
  pIKEClient->setMTU(517);

  // ---- NEW: search all services for our LED characteristic ----
  std::map<std::string, BLERemoteService *> *services = pIKEClient->getServices();
  if (!services || services->empty())
  {
    Serial.println("[IKE] No services found.");
    pIKEClient->disconnect();
    return false;
  }

  pIKECharLED = nullptr;
  for (auto const &it : *services)
  {
    BLERemoteService *svc = it.second;
    BLERemoteCharacteristic *ch = nullptr;
    try
    {
      ch = svc->getCharacteristic(IKE_CHAR_LED_UUID);
    }
    catch (...)
    {
      ch = nullptr;
    }

    if (ch != nullptr)
    {
      pIKECharLED = ch;
      Serial.print("[IKE] Found LED characteristic in service: ");
      Serial.println(svc->getUUID().toString().c_str());
      break;
    }
  }

  if (pIKECharLED == nullptr)
  {
    Serial.println("[IKE] LED characteristic not found in any service.");
    pIKEClient->disconnect();
    return false;
  }

  ikeConnected = true;
  Serial.println("[IKE] Connected & LED characteristic ready.");
  return true;
}

// ====== PUBLIC API ======

void ikeInit()
{
  // BLEDevice::init() must be called in main before this
  ikeConnected = false;
  ikeDoScan = true;
  ikeDoConnect = false;
}

void ikeLoop()
{
  // Try to connect if we already found device
  if (ikeDoConnect && !ikeConnected)
  {
    if (connectToIKE())
    {
      // Example: set red once on first connect
      ikeSetStatic(255, 0, 0, 0xFF);
    }
    else
    {
      Serial.println("[IKE] Connect failed, rescan.");
      ikeDoScan = true;
    }
    ikeDoConnect = false;
  }

  // Scan if not connected
  if (!ikeConnected && ikeDoScan)
  {
    Serial.println("[IKE] Scanning for I-KE-V3...");
    BLEScan *scan = BLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new IKEAdvertisedCallbacks());
    scan->setInterval(1349);
    scan->setWindow(449);
    scan->setActiveScan(true);
    scan->start(5, false); // non-blocking
  }
}

bool ikeSetStatic(int r, int g, int b, int brightness)
{
  uint8_t pkt[20];
  buildPacket(MODE_STATIC, r, g, b, brightness, 0x00, pkt);
  return sendPacketInternal(pkt);
}

bool ikeSetBreathe(int r, int g, int b, int speed)
{
  uint8_t pkt[20];
  buildPacket(MODE_BREATHE, r, g, b, speed, 0x00, pkt);
  return sendPacketInternal(pkt);
}

bool ikeSetBlink(int r, int g, int b, int speed)
{
  uint8_t pkt[20];
  buildPacket(MODE_BLINK, r, g, b, speed, 0x00, pkt);
  return sendPacketInternal(pkt);
}

bool ikeSendHexPacket(const String &hex)
{
  if (hex.length() != 40)
  {
    Serial.println("[IKE] Hex must be 40 characters (20 bytes).");
    return false;
  }
  uint8_t pkt[20];
  for (int i = 0; i < 20; ++i)
  {
    char buf[3] = {hex[2 * i], hex[2 * i + 1], 0};
    pkt[i] = (uint8_t)strtol(buf, nullptr, 16);
  }
  return sendPacketInternal(pkt);
}

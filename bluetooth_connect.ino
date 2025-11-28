#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- WROVER SAFE PINS ---
#define RX_PIN 26
#define TX_PIN 27
#define BAUDRATE 9600

// --- BLE UUIDs (SYNCHRONIZED WITH REACT SCRIPT) ---
// Service UUID expected by your web app:
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// Characteristic UUID expected by your web app (we will use this for TX/Notify/Write):
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8" 
// We will define a corresponding RX characteristic for the app to write to:
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" 

HardwareSerial SerialPIC(2);
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// --- 1. CALLBACKS FOR CONNECTION EVENTS ---
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      // DEBUG: Message confirming connection over USB Serial Monitor
      Serial.println("--- BLE Connection Successful! ---"); 

      // ************ SEND CONNECTED MESSAGE ************
      // Send confirmation to the connected Web App using the TX Characteristic.
      const char* connectMsg = "CONNECTED";
      pTxCharacteristic->setValue(connectMsg);
      pTxCharacteristic->notify();
      Serial.println("Sent 'CONNECTED' notification to Web App.");
      // *********************************************
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      // Re-initialize advertising immediately after disconnect
      Serial.println("Device disconnected. Starting advertising..."); 
    }
};

// --- 2. CALLBACKS FOR RECEIVING DATA FROM WEB APP (WRITE TO TX CHAR) ---
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      
      // Get the pointer to the raw data buffer and the length
      uint8_t* data = pCharacteristic->getData();
      size_t len = pCharacteristic->getLength();

      if (len > 0) {
        // Forward received BLE data to PIC via UART
        for (int i = 0; i < len; i++) {
          SerialPIC.write(data[i]);
          Serial.print((char)data[i]); // Debug: Print received character to USB
        }
      }
    }
};

// --- SETUP ---
void setup() {
  Serial.begin(9600);
  
  // Start UART connection to PIC on safe WROVER pins (26/27)
  SerialPIC.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Initialize BLE
  BLEDevice::init("ESP32_Voice_Bridge"); // This name appears in your web app scan list

  // Create the Server and set callbacks
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service (using the custom UUID 4fafc201...)
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the TX Characteristic (ESP32 -> Phone, using the custom UUID beb5483e...)
  // We combine all properties needed (NOTIFY, READ, WRITE) into the initial creation.
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY | 
                    BLECharacteristic::PROPERTY_READ |
                    BLECharacteristic::PROPERTY_WRITE
                  );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Set the callback for writing commands (App -> ESP32) on the main characteristic
  pTxCharacteristic->setCallbacks(new MyCallbacks());
  
  // Start the service and start advertising (making the ESP32 discoverable)
  pService->start();
  pServer->getAdvertising()->start();
  
  Serial.println("BLE Started! UUIDs synchronized with React App.");
}

// --- MAIN LOOP ---
void loop() {
  // --- A. PIC -> WEB APP (Transmit) ---
  if (deviceConnected && SerialPIC.available()) {
    // Read and buffer characters from PIC efficiently
    String str = "";
    while(SerialPIC.available()) {
      str += (char)SerialPIC.read();
      delay(2); // Small delay to gather characters into a single packet
    }
    
    // Send to Web App only if data was gathered
    if (str.length() > 0) {
      pTxCharacteristic->setValue((uint8_t*)str.c_str(), str.length());
      pTxCharacteristic->notify();
      Serial.print("Sent to App: "); Serial.println(str); // Debug: Confirm data transmission
    }
  }

  // --- B. CONNECTION MANAGEMENT ---
  if (!deviceConnected && oldDeviceConnected) {
      // Device just disconnected, restart advertising
      delay(500); 
      pServer->startAdvertising(); 
      oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
      // Device just connected
      oldDeviceConnected = deviceConnected;
      Serial.println("Device Connected to Web App!");
  }
  
  delay(1); // Yield to background tasks
}
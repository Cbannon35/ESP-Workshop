/*
  Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  updated by chegewara and MoThunderz
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Initialize all pointers
BLEServer* pServer = NULL;                        // Pointer to the server
BLECharacteristic* pCharacteristic_1 = NULL;      // Pointer to Characteristic 1
BLECharacteristic* pCharacteristic_2 = NULL;      // Pointer to Characteristic 2
BLEDescriptor *pDescr_1;                          // Pointer to Descriptor of Characteristic 1
BLE2902 *pBLE2902_1;                              // Pointer to BLE2902 of Characteristic 1
BLE2902 *pBLE2902_2;                              // Pointer to BLE2902 of Characteristic 2

// Some variables to keep track on device connected
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint16_t connId = -1;

// Variable that will continuously be increased and written to the client
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
// UUIDs used in this example:
#define SERVICE_UUID          "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_1 "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

// Callback function that is called whenever a client is connected or disconnected
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
      Serial.println("Client Connected");
      deviceConnected = true;
      connId = param->connect.conn_id;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Client Disconnected");
      deviceConnected = false;
      connId = -1;
    }
};

/* pChar1 */
class CharacteristicCallBack_1: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pchar) override {
    std::string stdstr = pchar->getValue();
    String recieved = String(stdstr.c_str());
    Serial.println(recieved);
  }
};

/* pChar2 */
class CharacteristicCallBack_2: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pchar) override {
    std::string stdstr = pchar->getValue();
    String recieved = String(stdstr.c_str());
    Serial.println(recieved);
    Serial.print(deviceConnected);
    Serial.print(" ");
    Serial.print(connId);
    if (deviceConnected && connId >= 0) {
      Serial.println("Disconnecting...?");
      pServer->disconnect(connId);
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic_1 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_1,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );                   

  pCharacteristic_2 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_2,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );  

  // Create a BLE Descriptor  
  pDescr_1 = new BLEDescriptor((uint16_t)0x2901);
  pDescr_1->setValue("A very interesting variable");
  pCharacteristic_1->addDescriptor(pDescr_1);

  // Add the BLE2902 Descriptor because we are using "PROPERTY_NOTIFY"
  pBLE2902_1 = new BLE2902();
  pBLE2902_1->setNotifications(true);                 
  pCharacteristic_1->addDescriptor(pBLE2902_1);
  pCharacteristic_1->setCallbacks(new CharacteristicCallBack_1());

  pBLE2902_2 = new BLE2902();
  pBLE2902_2->setNotifications(true);
  pCharacteristic_2->addDescriptor(pBLE2902_2);
  pCharacteristic_2->setCallbacks(new CharacteristicCallBack_2());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {
      // pCharacteristic_1 is an integer that is increased with every second
      // in the code below we send the value over to the client and increase the integer counter

      // pCharacteristic_2 is a std::string (NOT a String). In the code below we read the current value
      // write this to the Serial interface and send a different value back to the Client
      // Here the current value is read using getValue() 
      // Serial.println("We are connected to a client!");
    }
    // The code below keeps the connection status uptodate:
    // Disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // Connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


#define PRE_PIN          32 
#define VNOX_PIN         35 
#define VRED_PIN         34 

#define PRE_HEAT_SECONDS 10

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
int value = 0;


#define SERVICE_UUID        "c8a79586-930c-40af-af43-ea57c96ebfc3"
#define CHARACTERISTIC_UUID "18670649-822f-4db3-80e9-fdb2945fa641"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



int vnox_value = 0;
int vred_value = 0;

void setup() {
  
  pinMode(PRE_PIN, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("MiCS-4514 Test Read");
  Serial.print("Preheating...");
  
  digitalWrite(PRE_PIN, 1);
  delay(PRE_HEAT_SECONDS * 1000);
  digitalWrite(PRE_PIN, 0);
  Serial.println("Done");


  BLEDevice::init("ESP32");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); 
  BLEDevice::startAdvertising();
  Serial.println("Bilgi aktarmak için bir istemcinin bağlanması bekleniyor...");
}

void loop() {
  
  vnox_value = analogRead(VNOX_PIN);
  vred_value = analogRead(VRED_PIN);
  Serial.print("Vnox: ");
  Serial.print(vnox_value, DEC);
  Serial.print(" Vred: ");
  Serial.println(vred_value, DEC);


  //eğer bir istemci bağlanmışsa
  if (deviceConnected) {
    char gonderilecek[8];
    dtostrf(vred_value, 2, 1, gonderilecek); //float bir veriyi char dizisine çeviriyoruz
    pCharacteristic->setValue(gonderilecek);//bu komut ile hazırlanıp
    pCharacteristic->notify();//bu komut ile gönderiliyor
    Serial.println(gonderilecek);
    
    delay(1000);
  }

  // disconnect olma durumlarında
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // bluetooth katına biraz süre tanımak için
    pServer->startAdvertising(); // tekrar kendini yayınlamaya başlatalım
    Serial.println("Yayın başlatıldı");
    oldDeviceConnected = deviceConnected;
  }
  // Tekrar Bağlanma durumunda
  if (deviceConnected && !oldDeviceConnected) {
    // değişkenleri güncelle
    oldDeviceConnected = deviceConnected;
  }
}

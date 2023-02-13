
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

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


int veriPini = 4; // turuncu kablonun (sensörün soldan 2. pini) bağlı olduğu pin
float toz=0; //Gelen veriyi sakladığımız değişken
int led=2; // Beyaz kablonun (sensörün soldan 3. pini) bağlı olduğu pin

void setup() {
  
  Serial.begin(9600); 
  pinMode(led,OUTPUT);
  pinMode(veriPini, INPUT);


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
  
  digitalWrite(led,LOW); 
  delayMicroseconds(280);
  toz=analogRead(veriPini); 
  delayMicroseconds(40);
  digitalWrite(led,HIGH); 
  delayMicroseconds(9860);
  delay(1000);
  if (toz>36.455)
  Serial.println((float(toz/1024)-0.0356)*120000*0.035);
  
  if (deviceConnected) {
    char gonderilecek[8];
    dtostrf(toz, 2, 1, gonderilecek); //float bir veriyi char dizisine çeviriyoruz
    pCharacteristic->setValue(gonderilecek);//bu komut ile hazırlanıp
    pCharacteristic->notify();//bu komut ile gönderiliyor
    Serial.println(gonderilecek);
    
    delay(100);
  }

  // disconnect olma durumlarında
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); 
    pServer->startAdvertising(); 
    Serial.println("Yayın başlatıldı");
    oldDeviceConnected = deviceConnected;
  }
  // Tekrar Bağlanma durumunda
  if (deviceConnected && !oldDeviceConnected) {
    // değişkenleri güncelle
    oldDeviceConnected = deviceConnected;
  }
}
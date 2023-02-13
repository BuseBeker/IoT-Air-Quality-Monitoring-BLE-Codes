#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

//bağlı bağlıdeğil tanımlamaları
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


int Gas_analog = 4;    
int Gas_digital = 2;   

void setup() {
  
  Serial.begin(115200);    
  pinMode(Gas_digital, INPUT);


  BLEDevice::init("ESP32");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // BLE karakteristiğini oluşturalım
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
  
 int gassensorAnalog = analogRead(Gas_analog);
  int gassensorDigital = digitalRead(Gas_digital);

  Serial.print("Gas Sensor: ");
  Serial.print(gassensorAnalog);
  Serial.print("\t");
  Serial.print("Gas Class: ");
  Serial.print(gassensorDigital);
  Serial.print("\t");
  Serial.print("\t");
  
  if (gassensorAnalog > 1000) {
    Serial.println("Gas");
    delay(1000);
  }
  else {
    Serial.println("No Gas");
  }


  //eğer bir istemci bağlanmışsa
  if (deviceConnected) {
    char gonderilecek[8];
    dtostrf(gassensorAnalog, 2, 1, gonderilecek); //float bir veriyi char dizisine çeviriyoruz
    pCharacteristic->setValue(gonderilecek);//bu komut ile hazırlanıp
    pCharacteristic->notify();//bu komut ile gönderiliyor
    Serial.println(gonderilecek);
    
    delay(100);
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
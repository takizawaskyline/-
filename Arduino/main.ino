#define GH_INCLUDE_PORTAL
#include "WiFi.h"
#include <SPI.h>
#include <MFRC522.h>
#include <GyverMAX7219.h>
#include <RunningGFX.h>
#include <Pairs.h>
#include <PairsExt.h>
#include <PairsStatic.h>
#include "Arduino.h"
#include "GyverHub.h"

//============ Настройки пинов ============
#define CS_PIN 15      
#define DATA_PIN 4 
#define CLK_PIN 2 
#define RST_PIN 22  
#define SS_PIN 5  

#define HALL_PIN 34    
#define RELAY_PIN 21   
#define OPEN_TIMEOUT 10000 

//============ Настройки матрицы ============
#define M_WIDTH 2      
#define M_HEIGHT 1  
#define run_speed 15 
#define BRIGHT 4

//============ Объекты ============
MFRC522 rfid(SS_PIN, RST_PIN);
MAX7219 <M_WIDTH, M_HEIGHT, CS_PIN, DATA_PIN, CLK_PIN> mtrx; 
GyverHub hub("ZaharGroup", "Zahar_case", "");
RunningGFX run(&mtrx); 

Pairs balans;
String p_red = "Красных: ", p_white = "Белых: ", p_bright = "Бежевых: ";

//============ Переменные ============
bool flag [6] = {0, 0, 0, 0, 0, 0};
int color[3] = {0, 0, 0};
String str = "SmartToken";
String bal_str = "0";
int bal = 0;

uint32_t lockTimer = 0;
bool isLockOpen = false; 

//============ БАЗА КЛЮЧЕЙ ============
String masterKey = "01020304"; 

String white = "04130444819142129", white2 = "042412546319042129";
String br = "042252486319042129", br2 = "04239524819142129";
String red = "04206624819142129", red2 = "041322376319042129";

//============ ФУНКЦИИ ============

void initRFID() {
  digitalWrite(RST_PIN, LOW); 
  delay(50);
  digitalWrite(RST_PIN, HIGH);
  delay(50);
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max); 
  Serial.println("RFID: System Re-initialized");
}

// ФУНКЦИЯ ИНТЕРФЕЙСА (Исправлено под последнюю версию API)
void build(gh::Builder& b) {
  b.Title(F("Умный Сейф"));

  b.beginRow();
  // В новых версиях используется .label() для текста над виджетом
  b.Label(F("bal_widget")).label(F("Общий баланс")).value(str).size(2);
  b.endRow();

  b.beginRow();
  b.Label(F("c_white")).label(F("Белые (100)")).value(color[0]);
  b.Label(F("c_beige")).label(F("Бежевые (50)")).value(color[1]);
  b.Label(F("c_red")).label(F("Красные (10)")).value(color[2]);
  b.endRow();

  b.beginRow();
  // ИСПРАВЛЕНИЕ: Используем .label() для текста на самой кнопке
  if (b.Button().label(F("ОТКРЫТЬ ЗАМОК")).click()) {
    digitalWrite(RELAY_PIN, HIGH);
    lockTimer = millis();
    isLockOpen = true;
    Serial.println("HUB: LOCK OPENED MANUALLY");
  }
  b.endRow();
}

bool updateBal(int index, int nominal, int clr, String name) {
  bool added = false;
  if (flag[index] == 0) {
    bal += nominal;
    color[clr]++;
    flag[index] = 1;
    added = true;
    Serial.print("BALANCE: [+] ");
  } else {
    bal -= nominal;
    color[clr]--;
    flag[index] = 0;
    added = false;
    Serial.print("BALANCE: [-] ");
  }
  
  Serial.print(name);
  Serial.print(" | New Total: ");
  Serial.println(bal);

  bal_str = String(bal);
  str = bal_str + " $";
  run.setText(str); 
  
  // Обновление интерфейса в реальном времени
  hub.update("bal_widget").value(str);
  hub.update("c_white").value(color[0]);
  hub.update("c_beige").value(color[1]);
  hub.update("c_red").value(color[2]);
  
  return added;
}

//============ SETUP ============
void setup() {
  Serial.begin(115200); 
  delay(1000);
  Serial.println("\n\n================ SYSTEM START ================");
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); 
  pinMode(HALL_PIN, INPUT); 

  mtrx.begin(); 
  mtrx.setBright(BRIGHT);
  mtrx.setRotation(0); 
  mtrx.clear();
  run.setWindow(0, 16, 0); 
  run.setSpeed(run_speed);
  run.setText(str);
  run.start();

  SPI.begin();      
  initRFID();

  WiFi.mode(WIFI_STA);
  WiFi.begin("foresp", "12344321");
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    run.tick(); 
  }
  Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());

  hub.config(F("ZaharGyrkov"), F("Zahar_case"), F(""));
  hub.onBuild(build);
  hub.begin();
  
  Serial.println("==============================================");
}

//============ LOOP ============
void loop() {
  hub.tick();
  run.tick(); 

  if (isLockOpen) {
    if (analogRead(HALL_PIN) > 1500 || (millis() - lockTimer >= OPEN_TIMEOUT)) {
      digitalWrite(RELAY_PIN, LOW); 
      isLockOpen = false;
      Serial.println("LOCK: CLOSED");
    }
  }

  static uint32_t rfidTmr;
  if (millis() - rfidTmr >= 100) {
    rfidTmr = millis();

    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
      return; 
    }

    String cur = ""; 
    for (uint8_t i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) cur += "0";
      cur += String(rfid.uid.uidByte[i]); 
    }
    
    if (cur.length() < 4) {
      rfid.PICC_HaltA();
      return;
    }

    Serial.print("RFID UID: ");
    Serial.println(cur);

    bool shouldOpen = false;

    if (cur == masterKey) {
      shouldOpen = true;
      Serial.println("ACCESS: MASTER");
    } 
    else if (cur == white)  { if (updateBal(0, 100, 0, "White_1"))  shouldOpen = true; }
    else if (cur == white2) { if (updateBal(1, 100, 0, "White_2"))  shouldOpen = true; }
    else if (cur == br)     { if (updateBal(2, 50, 1, "Beige_1"))  shouldOpen = true; }
    else if (cur == br2)    { if (updateBal(3, 50, 1, "Beige_2"))  shouldOpen = true; }
    else if (cur == red)    { if (updateBal(4, 10, 2, "Red_1"))    shouldOpen = true; }
    else if (cur == red2)   { if (updateBal(5, 10, 2, "Red_2"))    shouldOpen = true; }
    else {
      Serial.println("ACCESS: UNKNOWN CARD");
    }

    if (shouldOpen) {
      digitalWrite(RELAY_PIN, HIGH);
      lockTimer = millis();
      isLockOpen = true;
    }
    
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

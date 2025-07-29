//============Подключение библиотек============\|
#include "WiFi.h"
#include <SPI.h>
#include <MFRC522.h>
#include <FastBot.h>
#include <GyverMAX7219.h>
#include <RunningGFX.h>
//===============================================\|


//============База данных ключей============\|
String white = "4130444819142129"; 
String white2 = "42412546319042129";
String br = "42252486319042129";
String br2 = "4239524819142129";
String red = "4206624819142129";
String red2 = "41322376319042129";
//==========================================\|


//============Настройки программы===========\|
#define CS 15 //Пин cs на светодиодной матрице
#define DATA 4 //Пин data(din) на светодиодной матрице
#define CLK 2 //Пин clk на светодиодной матрице
#define RST_PIN  22  // Пин RST RC522
#define SS_PIN   5  // Пин SDA (SS) RC522

#define ssid "" // Имя wifi сети
#define password "" //Пароль wifi сети
#define bot_token "" //Токен тг бота 

#define WIDTH 2 //Количевство матриц по горизонтали
#define HEIGHT 1 //Количевство матриц по вертикали
#define run_speed 15 // скорость бегущей строки
#define BRIGHT 4 //Яркость матрици
//=========================================\|
//ApoX51s42wR7FDK8



//============Настройка портов============\|
MFRC522 rfid(SS_PIN, RST_PIN);
MAX7219 < WIDTH, HEIGHT, CS, DATA , CLK > mtrx; 
//========================================\|

//============Инициализация============\|
FastBot bot(bot_token); //Подлючение к телеграм боту 
RunningGFX run(&mtrx); // Подключение матрици
//=====================================\|

//============Объявление переменных и массивов============\|
bool flag [6] = {0, 0, 0, 0, 0, 0};
int color[3] = {0, 0, 0};
String str = "Привет";
String bal_str = "0";
String color_balans;
String chatID;
int bal = 0;
//=========================================================\|


void setup() {

  Serial.begin(115200); //Открытие последовательного монитора порта на скорости 115200

//========================Матрица========================\|
  run.setSpeed(run_speed);
  mtrx.begin(); //Инициализация матрицы 
  mtrx.setBright(BRIGHT);  // яркость 0..15
  mtrx.clear();
  run.setText(str);
  run.start();
//========================================================\|

//============Инициализация============\|
  SPI.begin();      //Инициализация шины SPI
  rfid.PCD_Init();  //Инициализация RfID чситывателя
//=====================================\|

  wifi(); // Вызываем функцию подключения к WiFi
  bot.attach(newMsg) ; //Инициализация тг бота 
}
//==============================================\|


void loop() {
//============Тикаем в луп============\|
  run.tick();
  bot.tick();   
//====================================\|

  RfId(); //Вызываем функцию обработки меток 
} 

void newMsg(FB_msg& msg) {
  chatID = msg.userID; // Записываем в переменную id пользователя 

//============Отладка============\|
  Serial.print(msg.username);
  Serial.print(", ");
  Serial.println(msg.text);
  Serial.print(", ");
  Serial.print(msg.userID);
//===============================\|

//====================================Обработка сообщений====================================\|
if (msg.text == "/start") {
    bot.showMenu("Узнать баланс \n Запуск бота \n Количество монет", msg.userID);
  }

else if (msg.text == "Запуск бота") {
    bot.sendMessage("Бот запущен\n /bal - Узнать баланс \n /start - запустить бота (при включении обязательно)\n /coins - Узнать количевство монет", msg.userID);
  }

else if (msg.text == "Узнать баланс" || msg.text =="/bal") {
    bot.sendMessage("Балланс: " + bal_str + "💲", msg.userID);
  }

else if (msg.text == "Количество монет" || msg.text == "/coins") {
    int count =  color[0] + color[1] + color[2];
    color_balans =  "Всего монет: " + String(count) + "\n"+  
                    "Монеты в наличии:" + "\n" +
                    "⚪Белых: " + String(color[0]) + "\n" +
                    "🔴Красных: " + String(color[2]) + "\n" +
                    "🏻Бежевый: " + String(color[1]) + "\n";

    bot.sendMessage(color_balans, msg.userID);
  }

else { 
    bot.sendMessage("❌Ошибка. Такой команды не существует \n"
                      "Список команд: \n"
                      "/bal - Узнать баланс \n" 
                      "/start - запустить бота (при включении обязательно)\n"
                      "/coins - Узнать количевство монет",
                      msg.userID);
  }
}

//===========================================================================================\|

//====================================Обработкабаланса сейфа ====================================\|
void bal1(int index, int nominal, int clr) {

Serial.println(" ");
Serial.println(nominal);
  if (flag[index] == 0) {
        
        bal += nominal;
        color[clr]++;
        bal_str = String(bal);
        flag[index] = 1;
        delay(1000);
        bot.sendMessage("💸Пополнено: +" + String(nominal) + "💲\n"+ //Вывод данных при начеслении денег
                        "Баланс: " + bal_str + "💲", chatID);
    }
  else if (flag[index] == 1) {
    bal -= nominal;
    color[clr]--;
    bal_str = String(bal);
    flag[index] = 0;
    delay(1000);
    bot.sendMessage("💸Снято: -" + String(nominal) + "💲\n" + //Вывод данных при снятие денег
                    "Баланс: " + bal_str + "💲", chatID);
  }
    Serial.println();
    mtrx.clear();
    mtrx.setCursor(0, 0);
    Serial.println(" ");
    Serial.println(str);
    mtrx.update();
    str = String(bal) + "_$ ";
}
//===============================================================================================\|

void RfId () {
     if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      String cur_str; 
      for (uint8_t i = 0; i < rfid.uid.size; i++) {
        cur_str += rfid.uid.uidByte[i]; 
        Serial.print(" ");
      }
//========================Отладка======================\|
        Serial.println(" ");
        Serial.println("===================");
        Serial.println(cur_str);
//=====================================================\|

//====================================Проверка наличия монеты в базе====================================\|

       if (cur_str == white) {
         Serial.print("OK"); 
         bal1(0, 100,0);
       }

      else if (cur_str == white2) {
        Serial.print("OK"); 
         bal1(1, 100,0);
       }

      else if(cur_str == br){
         Serial.print("OK"); 
         bal1(2, 50, 1);
       }

      else if (cur_str == br2) {
         Serial.print("OK"); 
         bal1(3, 50,1);
       }

      else if(cur_str == red){
         Serial.print("OK"); 
         bal1(4, 10,2);
       }

      else if (cur_str == red2) {
         Serial.print("OK"); 
         bal1(5, 10,2);
       }
      
      
       else {
        Serial.print("ERR");
       }
//=========================================================================================================\|     
   }
}
//========================Функция подключения к wifi========================\|
void wifi() {
  delay(2000);
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() > 15000) ESP.restart();
  }
  Serial.println("Connected");
  Serial.println(WiFi.localIP());
}
//==========================================================================\|
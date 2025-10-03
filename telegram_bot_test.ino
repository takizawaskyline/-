#define WIFI_SSID "CPOD"
#define WIFI_PASS "ApoX51s42wR7FDK8"
#define BOT_TOKEN "8084535567:AAGx2dQeF8yXp4ZaYUqih6BsjNpSBfm_AY0"

#include <WiFi.h>
#include <FastBot.h>
FastBot bot(BOT_TOKEN);


void setup() {
  connectWiFi();
  bot.setChatID("");  
  bot.attach(newMsg);

}

void newMsg(FB_msg& msg) {
  // выводим имя юзера и текст сообщения
  Serial.print(msg.username);
  Serial.print(", ");
  Serial.println(msg.text);
  
  // выводим всю информацию о сообщении
  Serial.println(msg.toString());

  if (msg.text == "/start") {
    bot.showMenu("Меню_тест_1 \n Меню_тест_2 \n Меню_тест_3", msg.userID);
  }

  bot.sendMessage(msg.text + "<тест эхо бота>", msg.chatID);

}

void loop() {
  bot.tick();

}


void connectWiFi() {
  delay(2000);
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() > 15000) ESP.restart();
  }
  Serial.println("Connected");
}

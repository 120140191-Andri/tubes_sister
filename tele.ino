#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h> 
#include <Wire.h>
// Replace with your network credentials
const char* ssid = "!yesn'ts";
const char* password = ""; 
// Initialize Telegram BOT
#define BOTtoken "5905728552:AAH9nEa6pE7dCt8RLHPC4qqZq-tQ6XFvsQY"  // your Bot Token (Get from Botfather)
 
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
 
// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

//////
#include "DHT.h"
#define DHTPIN 4

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float t;

// sensor mq
#include "MQ135.h"
#define PIN_MQ135 39
float air_quality;

// LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Kipas
#define kipas 2

bool sub = true;

void kirimPesan(String pesan){
  String chat_id = "979590752";
  bot.sendMessage(chat_id, String(pesan), "");
}

void tampilLCD(){
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print(air_quality);
  lcd.setCursor(2, 1);
  lcd.print(t);
}

void bacaSensorMQ(){

  MQ135 gasSensor = MQ135(PIN_MQ135);
  air_quality = gasSensor.getPPM();
  float RS = gasSensor.getResistance();
  float R0 = gasSensor.getRZero();
  float AQ = analogRead(PIN_MQ135);
  delay(100);

  int ppmval = RS/R0;

  Serial.println(air_quality);

  if(air_quality > 0){
    if(sub){
      // kirimPesan("udara kotor");
    }
    digitalWrite(kipas, HIGH);
  }else{
    digitalWrite(kipas, LOW);
  }

  delay(100);

}

void bacaSensorDHT(){
  //  sensor kelembapan
 
   float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(f);
    Serial.print(F("°F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
    Serial.print(hif);
    Serial.println(F("°F"));

    if(t >= 30){
      digitalWrite(kipas, LOW);
    }else{
      digitalWrite(kipas, HIGH);
    }
}
 
// Handle what happens when you receive new messages
void bacaPesan(int numNewMessages) {
 
 Serial.println("bacaPesan");
 Serial.println(String(numNewMessages));
 
 for (int i=0; i<numNewMessages; i++) {
   // Chat id of the requester
   String chat_id = String(bot.messages[i].chat_id);
 
   // Print the received message
   String text = bot.messages[i].text;
   Serial.println(text);
 
   String from_name = bot.messages[i].from_name;
 
   if (text == "/start") {
     String welcome = "Welcome, " + from_name + ".\n";
     welcome += "Use the following commands to control your outputs.\n\n";
     welcome += "/led_on to turn GPIO ON \n";
     welcome += "/led_off to turn GPIO OFF \n";
     welcome += "/state to request current GPIO state \n";
     welcome += "/option to return the reply keyboard \n";
     bot.sendMessage(chat_id, welcome, "");
   }
 
   if (text == "led_on") {
     bot.sendMessage(chat_id, "LED state set to ON", "");
   }
 
   if (text == "led_off") {
     bot.sendMessage(chat_id, "LED state set to OFF", "");
   }

   if (text == "sub") {
     
     sub = !sub;

     if(sub){
       bot.sendMessage(chat_id, "Langganan diaktifkan", "");
     }else{
       bot.sendMessage(chat_id, "Langganan dimatikan", "");
     }
     
   }

   if (text == "nyalain kipas") {
     
    //  nyalain kipas
    delay(30000);
     
   }
 
   if (text == "/option") {
     String keyboardJson = "[[\"/led_on\", \"/led_off\"],[\"/state\"]]";
     bot.sendMessageWithReplyKeyboard(chat_id, "Choose one of the following options", "", keyboardJson, true);
   }
 }
}
 
void setup() {
 Serial.begin(115200);
 
 // Connect to Wi-Fi
 WiFi.begin(ssid, password);
 client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
 Serial.print("Connecting to WiFi");
 lcd.setCursor(0, 0);
 lcd.print("Connecting to WiFi");
  
 while (WiFi.status() != WL_CONNECTED) {
   delay(1000);
   Serial.print(".");
   lcd.setCursor(2, 1);
   lcd.print(".");
 }
 Serial.println();
 // Print ESP32 Local IP Address
 Serial.println(WiFi.localIP());
 
 lcd.begin();
 lcd.backlight();

 lcd.clear();

 pinMode(kipas, OUTPUT);
}
 
void loop() {

 if (millis() > lastTimeBotRan + botRequestDelay)  {
   int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

   while(numNewMessages) {
     Serial.println("got response");
     bacaPesan(numNewMessages);
     numNewMessages = bot.getUpdates(bot.last_message_received + 1);
   }

   lastTimeBotRan = millis();
 }

 bacaSensorMQ();
 bacaSensorDHT();
 tampilLCD();

}

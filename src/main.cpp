#include <Arduino.h>
#include "DHT.h"
#include "config.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define DHTPIN 19

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const unsigned long BOT_MTBS = 1000;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* bot_token = BOT_TOKEN;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);
unsigned long bot_lasttime;
bool Start = false;

void handleNewMessages(int numNewMessages)
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/send_test_action")
    {
      bot.sendChatAction(chat_id, "typing");
      delay(4000);
      bot.sendMessage(chat_id, "Did you see the action message?");
    }

    if (text == "/start")
    {
      String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
      welcome += "This is Chat Action Bot example.\n\n";
      welcome += "/send_test_action : to send test chat action message\n";
      bot.sendMessage(chat_id, welcome);
    }

    if (text == "/temperature")
    {

      float h = dht.readHumidity();
      float t = dht.readTemperature();
      float f = dht.readTemperature(true);

      if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
      }

      float hic = dht.computeHeatIndex(t, h, false);

      String msg = "Here are the tempeature details, " + from_name + "\n\n";
      msg += "Humidity: " + String(h) + "%\n";
      msg += "Temperature: " + String(t) + "°C\n";
      msg += "Heat Index: " + String(hic) + "°C\n";
      Serial.print(h);

      bot.sendMessage(chat_id, msg);
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  dht.begin();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}

void loop() {

  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    } 
  }
}
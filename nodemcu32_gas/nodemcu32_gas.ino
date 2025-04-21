String incomingData = "";
float temperature = 0.0;
float humidity = 0.0;
float ppm = 0.0;

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Wifi network station credentials
#define WIFI_SSID "Zombie"
#define WIFI_PASSWORD "23042001"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "8054565196:AAGy8L9938jyoPLuyNdR-n7oxfEGAIJ0JSQ"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;          // last time messages' scan has been done
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
      String welcome = "PPM = " + String(ppm) + "\n";
      welcome += "Kelembaban = " + String(humidity) + "\n";
      welcome += "Suhu = " + String(temperature) + "\n";
      bot.sendMessage(chat_id, welcome);
    }

    if (text == "/start")
    {
      String welcome = "PPM = " + String(ppm) + "\n";
      welcome += "Kelembaban = " + String(humidity) + "\n";
      welcome += "Suhu = " + String(temperature) + "\n";
      bot.sendMessage(chat_id, welcome);
    }
    if (text == "/monitor")
    {
      String welcome = "PPM = " + String(ppm) + "\n";
      welcome += "Kelembaban = " + String(humidity) + "\n";
      welcome += "Suhu = " + String(temperature) + "\n";
      bot.sendMessage(chat_id, welcome);
    }
  }
}


void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(9600);
  Serial2.begin(9600); // RX2 = pin 17, TX2 = pin 16

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(2, LOW);
    Serial.print(".");
    delay(500);
    digitalWrite(2, HIGH);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  for (int i = 0; i < 10; i++) {
    digitalWrite(2, LOW);
    delay(10);
    digitalWrite(2, HIGH);
    delay(10);
  }
  Serial.println(now);
}
bool kunci = 0;
void loop() {
  if (Serial2.available()) {
    incomingData = Serial2.readStringUntil('\n');  // Baca data sampai newline
    parseData(incomingData);

    // Tampilkan hasil parsing ke Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidity: ");
    Serial.print(humidity);
    Serial.print(" % | PPM: ");
    Serial.println(ppm);
  }
  if (ppm > 150 && kunci == 0) {
    kunci = 1;
    String welcome = "PPM = " + String(ppm) + "\n";
    welcome += "Kelembababn = " + String(humidity) + "\n";
    welcome += "Suhu = " + String(temperature) + "\n";
    bot.sendMessage("1236154265", welcome);
  }
  else if (ppm < 50 && kunci == 1) {
    kunci = 0;
    String welcome = "Situasi Sudah Normal";
    bot.sendMessage("1236154265", welcome);
  }
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}

// Fungsi untuk parsing data string dan simpan ke variabel
void parseData(String data) {
  int tIndex = data.indexOf("T:");
  int hIndex = data.indexOf(",H:");
  int ppmIndex = data.indexOf(",PPM:");

  if (tIndex != -1 && hIndex != -1 && ppmIndex != -1) {
    String tStr = data.substring(tIndex + 2, hIndex);
    String hStr = data.substring(hIndex + 3, ppmIndex);
    String ppmStr = data.substring(ppmIndex + 5);

    temperature = tStr.toFloat();
    humidity = hStr.toFloat();
    ppm = ppmStr.toFloat();
  }
}

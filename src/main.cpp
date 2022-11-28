#include <Arduino.h>
#include <time.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define WIFI_SSID "3 trước"
#define WIFI_PASSWORD "68686868"

#define MQTT_SERVER "192.168.0.6"
#define MQTT_PORT 1883

#define DHT_PORT 5
#define DHT_TYPE DHT11

#define MQ2_PIN A0

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const String CHIP_ID = String((unsigned long)((ESP.getEfuseMac() & 0xFFFF0000) >> 16), HEX) + String((unsigned long)((ESP.getEfuseMac() & 0x0000FFFF)), HEX);

const String TOPIC_DATA = CHIP_ID + "/data";
const String TOPIC_CONTROL = CHIP_ID + "/control";

DHT dht(DHT_PORT, DHT_TYPE);

const int SEND_JSON_CAPACITY = JSON_OBJECT_SIZE(12);
StaticJsonDocument<SEND_JSON_CAPACITY> doc;
JsonObject data = doc.createNestedObject("data");

const int RECEIVE_JSON_CAPACITY = JSON_OBJECT_SIZE(2);
StaticJsonDocument<RECEIVE_JSON_CAPACITY> rdoc;

char buffer[256];
unsigned long current_time, sending_cycle = 2000;

char *stringToChar(String string)
{
  int string_len = string.length() + 1;
  char *res = (char *)malloc(string_len);
  string.toCharArray(res, string_len);

  return res;
}

void setup_wifi()
{
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connect_to_broker()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      client.subscribe(stringToChar(TOPIC_CONTROL));
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  char status[256] = {};
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  for (int i = 0; i < length; i++)
  {
    status[i] = payload[i];
  }

  DeserializationError err = deserializeJson(rdoc, status);

  if (err)
  {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.f_str());
  }
  else
  {
    const char *type = rdoc["type"];
    const char *payload = rdoc["payload"];

    Serial.print("type: ");
    Serial.print(type);
    Serial.print(" - payload: ");
    Serial.println(payload);

    if (!strcmp(type, "rate"))
    {
      Serial.print("Change rate from ");
      Serial.print(sending_cycle);
      Serial.print(" to ");

      sending_cycle = atoi(payload);

      Serial.println(sending_cycle);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(500);

  delay(3000);

  Serial.println("Running.....");

  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
  connect_to_broker();
  Serial.println("Start MQTT client");

  doc["device_id"] = CHIP_ID;
  doc["time"] = 1351824120;
}

void loop()
{
  int mq2_value = analogRead(A0);
  data["gas"] = mq2_value;

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  data["temperature"] = String(temperature, 2);
  data["humidity"] = String(humidity, 2);

  // some fake datas
  data["CO"] = "0.00";
  data["NO2"] = "0.00";
  data["SO2"] = "0.00";
  data["PM2.5"] = "0.00";
  data["pressure"] = "0";
  data["fog"] = "0.00";

  serializeJsonPretty(doc, buffer);

  if (!client.connected()) // Kiểm tra kết nối
    connect_to_broker();

  client.loop();

  if (millis() - current_time > sending_cycle) // nếu 500 mili giây trôi qua
  {
    current_time = millis();
    Serial.println(buffer);
    client.publish(stringToChar(TOPIC_DATA), buffer); // gửi dữ liệu lên topic IoT47_MQTT_Test
  }
}
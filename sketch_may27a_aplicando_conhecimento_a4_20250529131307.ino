#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- Wi-Fi ---
const char* ssid = "Leonardo cel";
const char* password = "11223344";

// --- Broker MQTT ---
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "temperatura/agua";  // Corrigido para refletir o tópico real

// --- Pinos ---
#define ONE_WIRE_BUS 4
#define LED_PIN 5

WiFiClient espClient;
PubSubClient client(espClient);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado. IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado ao broker MQTT.");
    } else {
      Serial.print("Falha. rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  sensors.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC != DEVICE_DISCONNECTED_C) {
    Serial.print("Temperatura: ");
    Serial.println(tempC);

    String payload = "{\"temperatura\":";
    payload += tempC;
    payload += ",\"timestamp\":\"";
    payload += millis() / 1000;
    payload += "\"}";

    Serial.print("Enviando MQTT: ");
    Serial.println(payload);
    client.publish(mqtt_topic, payload.c_str());

    if (tempC >= 35.0 && tempC <= 45.0) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  } else {
    Serial.println("Erro: Sensor não detectado!");
  }

  delay(5000); // A cada 5 segundos
}
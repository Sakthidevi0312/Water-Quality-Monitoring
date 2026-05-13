#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi settings
const char* ssid = "Airel_9842878776";
const char* password = "air8858";

// MQTT settings
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* clientID = "ESP32_WaterMonitor";

// MQTT topics
const char* topic_ph = "watermonitor/ph";
const char* topic_flow = "watermonitor/flow";
const char* topic_level = "watermonitor/level";
const char* topic_tds = "watermonitor/tds";
const char* topic_turbidity = "watermonitor/turbidity";  

// Sensor pins
#define TRIG_PIN 01
#define ECHO_PIN 03
#define FLOW_SENSOR_PIN 23
#define PH_SENSOR_PIN 34
#define TDS_SENSOR_PIN 35
#define TURBIDITY_SENSOR_PIN 32  

// Flow sensor variables
volatile int pulseCount = 0;
float flowRate = 0;
float totalLitres = 0;
unsigned long lastTime = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void IRAM_ATTR countPulse() {
  pulseCount++;
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect(clientID)) {
      Serial.println("Connected to MQTT!");
    } else {
      Serial.print(".");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  connectWiFi();

  mqttClient.setServer(mqttServer, mqttPort);
  connectMQTT();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countPulse, RISING);
  analogReadResolution(12); // 12-bit ADC resolution
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  // --- Water Level ---
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float level = duration * 0.034 / 2;
  Serial.print("Level: "); Serial.println(level);
  mqttClient.publish(topic_level, String(level, 1).c_str());

  // --- Flow Rate ---
  if (millis() - lastTime > 1000) {
    detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN));
    flowRate = pulseCount / 4.5;
    totalLitres += flowRate / 60.0;
    Serial.print("Flow: "); Serial.print(flowRate); Serial.println(" L/min");
    mqttClient.publish(topic_flow, String(flowRate, 2).c_str());
    pulseCount = 0;
    lastTime = millis();
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countPulse, RISING);
  }

  // --- pH Sensor ---
  int adc_ph = analogRead(PH_SENSOR_PIN);
  float voltage_ph = adc_ph * (3.3 / 4095.0);
  float pH = 7 + ((2.5 - voltage_ph) / 0.18);
  Serial.print("pH: "); Serial.println(pH);
  mqttClient.publish(topic_ph, String(pH, 2).c_str());

  // --- TDS Sensor ---
  int adc_tds = analogRead(TDS_SENSOR_PIN);
  float voltage_tds = adc_tds * (3.3 / 4095.0);
  float tds = (133.42 * voltage_tds * voltage_tds * voltage_tds 
               - 255.86 * voltage_tds * voltage_tds 
               + 857.39 * voltage_tds) * 0.5;
  Serial.print("TDS: "); Serial.print(tds); Serial.println(" ppm");
  mqttClient.publish(topic_tds, String(tds, 1).c_str());

  // --- Turbidity Sensor ---
  int adc_turb = analogRead(TURBIDITY_SENSOR_PIN);
  float voltage_turb = adc_turb * (3.3 / 4095.0);
  float turbidity;
  if (voltage_turb < 2.5) {
    turbidity = 3000;  // Max NTU for dark water
  } else {
    turbidity = -1120.4 * sq(voltage_turb) + 5742.3 * voltage_turb - 4352.9;
    turbidity = fmax(0.0, turbidity); // Avoid negative values
  }
  Serial.print("Turbidity: "); Serial.print(turbidity); Serial.println(" NTU");
  mqttClient.publish(topic_turbidity, String(turbidity, 1).c_str());

  delay(1000);
}

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ⚠️ Wi-Fi / MQTT 설정
const char* WIFI_SSID     = "여기에_SSID";
const char* WIFI_PASSWORD = "여기에_PW";
const char* MQTT_BROKER   = "라즈베리파이_IP";  // 예: "192.168.0.10"
const int   MQTT_PORT     = 1883;
const char* TOPIC         = "glove/left";

MPU6050 mpu;
WiFiClient espClient;
PubSubClient mqtt(espClient);

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Wi-Fi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi 연결 성공!");
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("MQTT 연결 중...");
    if (mqtt.connect("left_glove")) {
      Serial.println("✅ MQTT 연결 성공!");
    } else {
      Serial.print("❌ 실패, 재시도...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("❌ MPU-6050 연결 실패!");
    while (1);
  }
  Serial.println("✅ MPU-6050 연결 성공!");

  connectWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float accel_x = ax / 16384.0;
  float accel_y = ay / 16384.0;
  float accel_z = az / 16384.0;

  float pitch = atan2(accel_y, accel_z) * 180.0 / PI;
  float roll  = atan2(-accel_x, accel_z) * 180.0 / PI;

  // JSON 형태로 전송
  char payload[64];
  snprintf(payload, sizeof(payload),
           "{\"pitch\":%.2f,\"roll\":%.2f,\"az\":%.2f}",
           pitch, roll, accel_z);

  mqtt.publish(TOPIC, payload);
  Serial.println(payload);

  delay(100);  // 10Hz
}
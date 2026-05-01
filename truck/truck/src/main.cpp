#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ⚠️ Wi-Fi / MQTT 설정
const char* WIFI_SSID     = "여기에_SSID";
const char* WIFI_PASSWORD = "여기에_PW";
const char* MQTT_BROKER   = "라즈베리파이_IP";
const int   MQTT_PORT     = 1883;

// TB6612FNG 핀 설정
#define AIN1 25
#define AIN2 26
#define PWMA 27
#define BIN1 14
#define BIN2 12
#define PWMB 13

// 제스처 임계값
#define PITCH_FWD    15.0   // Pitch > 15° → 전진
#define PITCH_BWD   -15.0   // Pitch < -15° → 후진
#define ROLL_RIGHT   15.0   // Roll > 15° → 우회전
#define ROLL_LEFT   -15.0   // Roll < -15° → 좌회전
#define STOP_AZ      0.3    // 양손 az < 0.3 → 정지 (손 위로)

#define MOTOR_SPEED 150     // PWM 0~255

// 장갑 데이터 저장
float left_pitch = 0, left_roll = 0, left_az = 1;
float right_pitch = 0, right_roll = 0, right_az = 1;

WiFiClient espClient;
PubSubClient mqtt(espClient);

// ===== 모터 제어 함수 =====
void motorStop() {
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW); analogWrite(PWMA, 0);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW); analogWrite(PWMB, 0);
  Serial.println("🛑 정지");
}

void motorForward(int speed) {
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); analogWrite(PWMA, speed);
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); analogWrite(PWMB, speed);
  Serial.println("⬆️ 전진");
}

void motorBackward(int speed) {
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); analogWrite(PWMA, speed);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); analogWrite(PWMB, speed);
  Serial.println("⬇️ 후진");
}

void motorLeft(int speed) {
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); analogWrite(PWMA, speed);
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); analogWrite(PWMB, speed);
  Serial.println("⬅️ 좌회전");
}

void motorRight(int speed) {
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); analogWrite(PWMA, speed);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); analogWrite(PWMB, speed);
  Serial.println("➡️ 우회전");
}

// ===== 규칙 기반 제스처 판단 =====
void applyGestureRule() {
  // 양손 위로 → 정지 (최우선)
  if (left_az < STOP_AZ && right_az < STOP_AZ) {
    motorStop();
    return;
  }

  // 전진 / 후진 (pitch 기준)
  if (left_pitch > PITCH_FWD) {
    motorForward(MOTOR_SPEED);
  } else if (left_pitch < PITCH_BWD) {
    motorBackward(MOTOR_SPEED);
  }
  // 좌회전 / 우회전 (roll 기준)
  else if (left_roll < ROLL_LEFT) {
    motorLeft(MOTOR_SPEED);
  } else if (left_roll > ROLL_RIGHT) {
    motorRight(MOTOR_SPEED);
  } else {
    motorStop();
  }
}

// ===== MQTT 수신 콜백 =====
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';

  StaticJsonDocument<128> doc;
  deserializeJson(doc, payload);

  if (String(topic) == "glove/left") {
    left_pitch = doc["pitch"];
    left_roll  = doc["roll"];
    left_az    = doc["az"];
  } else if (String(topic) == "glove/right") {
    right_pitch = doc["pitch"];
    right_roll  = doc["roll"];
    right_az    = doc["az"];
  }

  applyGestureRule();
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi 연결!");
}

void connectMQTT() {
  while (!mqtt.connected()) {
    if (mqtt.connect("truck")) {
      mqtt.subscribe("glove/left");
      mqtt.subscribe("glove/right");
      Serial.println("✅ MQTT 연결!");
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // 모터 핀 설정
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(PWMB, OUTPUT);
  motorStop();

  connectWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();
}
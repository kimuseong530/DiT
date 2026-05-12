#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ⚠️ Wi-Fi / MQTT 설정
const char* WIFI_SSID     = "여기에_SSID";
const char* WIFI_PASSWORD = "여기에_PW";
const char* MQTT_BROKER   = "라즈베리파이_IP";
const int   MQTT_PORT     = 1883;

// TB6612FNG 핀
#define AIN1 25
#define AIN2 26
#define PWMA 27
#define BIN1 14
#define BIN2 12
#define PWMB 13

// 인코더 핀
#define ENC_L_A 18
#define ENC_L_B 19
#define ENC_R_A 34
#define ENC_R_B 35

// 인코더 스펙
#define PPR          11       // 모터 1회전당 펄스 수
#define GEAR_RATIO   298      // 기어비
#define WHEEL_RADIUS 0.03     // 바퀴 반지름 (미터) ← 실측 후 수정!

// 제스처 임계값
#define PITCH_FWD    15.0
#define PITCH_BWD   -15.0
#define ROLL_RIGHT   15.0
#define ROLL_LEFT   -15.0
#define STOP_AZ      0.3
#define MOTOR_SPEED  150

// ===== 인코더 변수 =====
volatile long encL_count = 0;  // 왼쪽 누적 펄스
volatile long encR_count = 0;  // 오른쪽 누적 펄스

float distL_total = 0;  // 왼쪽 총 이동거리 (m)
float distR_total = 0;  // 오른쪽 총 이동거리 (m)
float heading = 0;      // 현재 방향각 (°)

// 바퀴 간격 (미터) ← 실측 후 수정!
#define WHEEL_BASE 0.15

// 인코더 펄스 → 거리 변환 계수
// 1펄스당 이동거리 = (2π × 반지름) / (PPR × 기어비)
const float DIST_PER_PULSE = (2.0 * PI * WHEEL_RADIUS) / (PPR * GEAR_RATIO);

// ===== 인터럽트 핸들러 =====
void IRAM_ATTR encL_ISR() {
  if (digitalRead(ENC_L_B) == HIGH) encL_count++;
  else encL_count--;
}

void IRAM_ATTR encR_ISR() {
  if (digitalRead(ENC_R_B) == HIGH) encR_count++;
  else encR_count--;
}

// ===== 장갑 데이터 =====
float left_pitch = 0, left_roll = 0, left_az = 1;
float right_pitch = 0, right_roll = 0, right_az = 1;

WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastSensorPublish = 0;
#define SENSOR_INTERVAL 100  // 100ms마다 센서 발행

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

// ===== 인코더 → 거리/방향 계산 =====
void updateOdometry() {
  // 현재 펄스 읽고 초기화
  long countL = encL_count;
  long countR = encR_count;
  encL_count = 0;
  encR_count = 0;

  // 거리 계산
  float dL = countL * DIST_PER_PULSE;
  float dR = countR * DIST_PER_PULSE;

  distL_total += abs(dL);
  distR_total += abs(dR);

  // 방향각 계산
  float dTheta = (dR - dL) / WHEEL_BASE;
  heading += dTheta * (180.0 / PI);

  // 360도 범위로 정규화
  if (heading > 360) heading -= 360;
  if (heading < 0)   heading += 360;
}

// ===== 센서 데이터 MQTT 발행 =====
void publishSensorData() {
  updateOdometry();

  float avgDist = (distL_total + distR_total) / 2.0;

  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"dist\":%.4f,\"distL\":%.4f,\"distR\":%.4f,\"heading\":%.2f}",
           avgDist, distL_total, distR_total, heading);

  mqtt.publish("truck/sensor", payload);
  Serial.println(payload);
}

// ===== 규칙 기반 제스처 판단 =====
void applyGestureRule() {
  if (left_az < STOP_AZ && right_az < STOP_AZ) {
    motorStop(); return;
  }
  if (left_pitch > PITCH_FWD)        motorForward(MOTOR_SPEED);
  else if (left_pitch < PITCH_BWD)   motorBackward(MOTOR_SPEED);
  else if (left_roll < ROLL_LEFT)    motorLeft(MOTOR_SPEED);
  else if (left_roll > ROLL_RIGHT)   motorRight(MOTOR_SPEED);
  else                               motorStop();
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

  // 모터 핀
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(PWMB, OUTPUT);
  motorStop();

  // 인코더 핀
  pinMode(ENC_L_A, INPUT_PULLUP);
  pinMode(ENC_L_B, INPUT_PULLUP);
  pinMode(ENC_R_A, INPUT_PULLUP);
  pinMode(ENC_R_B, INPUT_PULLUP);

  // 인터럽트 등록
  attachInterrupt(digitalPinToInterrupt(ENC_L_A), encL_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), encR_ISR, RISING);

  connectWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  // 100ms마다 센서 데이터 발행
  if (millis() - lastSensorPublish >= SENSOR_INTERVAL) {
    publishSensorData();
    lastSensorPublish = millis();
  }
}
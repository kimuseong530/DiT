#include <Arduino.h>

// ========== 앞R 모터 핀 정의 ==========
#define BIN1_FR  5
#define BIN2_FR  17
#define PWMB_FR  16

// 인코더 핀
#define ENC_A_FR  21
#define ENC_B_FR  22

// LEDC PWM 설정
#define CH_FR     0
#define PWM_FREQ  5000
#define PWM_BIT   8

// 인코더 카운트 변수
volatile long encoderCount = 0;

// 인코더 인터럽트 함수
void IRAM_ATTR encoderISR() {
  if (digitalRead(ENC_B_FR) == HIGH) {
    encoderCount++;
  } else {
    encoderCount--;
  }
}

void setup() {
  Serial.begin(115200);

  // 모터 방향 핀
  pinMode(BIN1_FR, OUTPUT);
  pinMode(BIN2_FR, OUTPUT);

  // 인코더 핀
  pinMode(ENC_A_FR, INPUT);
  pinMode(ENC_B_FR, INPUT);

  // 인코더 인터럽트 등록
  attachInterrupt(digitalPinToInterrupt(ENC_A_FR), encoderISR, RISING);

  // PWM 채널 설정
  ledcSetup(CH_FR, PWM_FREQ, PWM_BIT);
  ledcAttachPin(PWMB_FR, CH_FR);

  Serial.println("=== 앞R 모터 테스트 시작 ===");
  delay(1000);

  // ① 전진
  Serial.println("[1] 전진 2초");
  encoderCount = 0;
  digitalWrite(BIN1_FR, HIGH);
  digitalWrite(BIN2_FR, LOW);
  ledcWrite(CH_FR, 150);
  delay(2000);

  ledcWrite(CH_FR, 0);
  Serial.print("인코더 카운트: ");
  Serial.println(encoderCount);
  delay(1000);

  // ② 후진
  Serial.println("[2] 후진 2초");
  encoderCount = 0;
  digitalWrite(BIN1_FR, LOW);
  digitalWrite(BIN2_FR, HIGH);
  ledcWrite(CH_FR, 150);
  delay(2000);

  ledcWrite(CH_FR, 0);
  Serial.print("인코더 카운트: ");
  Serial.println(encoderCount);
  delay(1000);

  // ③ 속도 단계별 테스트
  Serial.println("[3] 속도 단계별 테스트");
  digitalWrite(BIN1_FR, HIGH);
  digitalWrite(BIN2_FR, LOW);

  int speeds[] = {80, 120, 180, 255};
  for (int i = 0; i < 4; i++) {
    Serial.print("속도: ");
    Serial.println(speeds[i]);
    ledcWrite(CH_FR, speeds[i]);
    delay(1500);
  }

  ledcWrite(CH_FR, 0);
  Serial.println("=== 테스트 완료 ===");
}

void loop() {
  // 시리얼 모니터에서 실시간 인코더 값 확인
  Serial.print("인코더: ");
  Serial.println(encoderCount);
  delay(200);
}
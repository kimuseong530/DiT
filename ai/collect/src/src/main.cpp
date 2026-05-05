#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// 제스처 라벨
// 0: 전진, 1: 후진, 2: 좌회전, 3: 우회전, 4: 정지
int currentLabel = 0;

// 한 샘플당 시계열 길이
#define SAMPLE_SIZE 20
#define SAMPLE_DELAY 50  // ms

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("❌ MPU6050 연결 실패!");
    while (1);
  }

  Serial.println("✅ 준비 완료!");
  Serial.println("라벨 입력 → 0:전진 1:후진 2:좌회전 3:우회전 4:정지");
  Serial.println("원하는 숫자 입력 후 엔터 → 수집 시작");
}

void collectSample() {
  Serial.println("3초 후 수집 시작...");
  delay(3000);

  for (int i = 0; i < SAMPLE_SIZE; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    float accel_x = ax / 16384.0;
    float accel_y = ay / 16384.0;
    float accel_z = az / 16384.0;
    float gyro_x  = gx / 131.0;
    float gyro_y  = gy / 131.0;
    float gyro_z  = gz / 131.0;

    float pitch = atan2(accel_y, accel_z) * 180.0 / PI;
    float roll  = atan2(-accel_x, accel_z) * 180.0 / PI;

    // CSV 형식으로 출력 → PC에서 저장
    Serial.print(pitch); Serial.print(",");
    Serial.print(roll);  Serial.print(",");
    Serial.print(accel_x); Serial.print(",");
    Serial.print(accel_y); Serial.print(",");
    Serial.print(accel_z); Serial.print(",");
    Serial.print(gyro_x); Serial.print(",");
    Serial.print(gyro_y); Serial.print(",");
    Serial.print(gyro_z); Serial.print(",");
    Serial.println(currentLabel);

    delay(SAMPLE_DELAY);
  }
  Serial.println("✅ 샘플 수집 완료!");
}

void loop() {
  if (Serial.available()) {
    char input = Serial.read();
    if (input >= '0' && input <= '4') {
      currentLabel = input - '0';
      String labels[] = {"전진","후진","좌회전","우회전","정지"};
      Serial.print("라벨: "); Serial.println(labels[currentLabel]);
      collectSample();
    }
  }
}
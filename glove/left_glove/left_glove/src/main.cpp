#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin();  // SDA=21, SCL=22

  Serial.println("MPU-6050 초기화 중...");
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("Connection successful");
  } else {
    Serial.println("no connection");
    while (1);
  }
}

void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float accel_x = ax / 16384.0;
  float accel_y = ay / 16384.0;
  float accel_z = az / 16384.0;

  float gyro_x = gx / 131.0;
  float gyro_y = gy / 131.0;
  float gyro_z = gz / 131.0;

  float pitch = atan2(accel_y, accel_z) * 180.0 / PI;
  float roll  = atan2(-accel_x, accel_z) * 180.0 / PI;

  Serial.println("============================");
  Serial.print("Pitch: "); Serial.print(pitch); Serial.println("°");
  Serial.print("Roll : "); Serial.print(roll);  Serial.println("°");
  Serial.println("---");
  Serial.print("가속도 X: "); Serial.print(accel_x); Serial.println(" g");
  Serial.print("가속도 Y: "); Serial.print(accel_y); Serial.println(" g");
  Serial.print("가속도 Z: "); Serial.print(accel_z); Serial.println(" g");
  Serial.println("---");
  Serial.print("자이로 X: "); Serial.print(gyro_x); Serial.println(" °/s");
  Serial.print("자이로 Y: "); Serial.print(gyro_y); Serial.println(" °/s");
  Serial.print("자이로 Z: "); Serial.print(gyro_z); Serial.println(" °/s");

  delay(500);
}
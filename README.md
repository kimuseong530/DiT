# DiT — Digital Twin Truck

> 손 제스처로 조종하는 소형 트럭 + Unity 디지털 트윈 실시간 동기화 시스템

---

## 프로젝트 개요
장갑에 내장된 IMU 센서로 손의 움직임을 인식하고,
Wi-Fi(MQTT) 통신을 통해 3D 프린팅 기반 소형 트럭을 제어합니다.
트럭의 실시간 위치·자세 데이터는 Unity 가상환경에 동기화되어
디지털 트윈으로 시각화됩니다.

---

## 시스템 구성

[ 장갑 (ESP32 + MPU-6050) ]
        ↓ MQTT (glove/left, glove/right)
[ Raspberry Pi — Mosquitto 브로커 ]
        ↓ MQTT (truck/cmd, truck/sensor)
[ 트럭 ESP32 — TB6612FNG — DC모터 ]
        ↓ MQTT (truck/sensor)
[ Unity 디지털 트윈 ]

---

## 제스처 동작 매핑

| 동작 | 기능 | 인식 방법 |
|---|---|---|
| 팔을 앞으로 밀기 | 전진 | Pitch + |
| 팔을 뒤로 당기기 | 후진 | Pitch − |
| 왼쪽으로 기울임 | 좌회전 | Roll − |
| 오른쪽으로 기울임 | 우회전 | Roll + |
| 양손 위로 | 정지 | Z축 기준 |

---

## MQTT 토픽 구조

| 토픽 | 발행자 | 구독자 | 내용 |
|---|---|---|---|
| `glove/left` | 왼손 ESP32 | 트럭, Unity | pitch/roll/yaw |
| `glove/right` | 오른손 ESP32 | 트럭, Unity | pitch/roll/yaw |
| `truck/cmd` | 트럭 ESP32 | Unity | 모터 명령값 |
| `truck/sensor` | 트럭 ESP32 | Unity | 인코더 피드백 |

---

## 하드웨어 구성
- ESP32 DevKitC WROOM-32D × 3
- MPU-6050 (GY-521) × 2
- TB6612FNG 모터드라이버 × 1
- 인코더 내장 DC모터 (6V 52RPM) × 2
- Raspberry Pi 4 Model B (MQTT 브로커)

---

## 실행 방법

### 1. Raspberry Pi — MQTT 브로커
```bash
cd raspberry_pi
bash setup.sh
```

### 2. ESP32 — 장갑 / 트럭
- `secrets.h`에 Wi-Fi SSID/PW 입력
- Arduino IDE에서 각 `.ino` 업로드

### 3. Unity
- `unity/` 폴더를 Unity Hub에서 열기
- MQTT 브로커 IP 설정 후 실행

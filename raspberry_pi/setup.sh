#!/bin/bash

echo "============================="
echo " Mosquitto MQTT 브로커 설치"
echo "============================="

# 패키지 업데이트
sudo apt update
sudo apt upgrade -y

# Mosquitto 설치
sudo apt install -y mosquitto mosquitto-clients

# 설정 파일 복사
sudo cp mosquitto.conf /etc/mosquitto/mosquitto.conf

# 부팅 시 자동 시작 설정
sudo systemctl enable mosquitto
sudo systemctl restart mosquitto

# 실행 상태 확인
echo "============================="
echo " Mosquitto 실행 상태"
echo "============================="
sudo systemctl status mosquitto

# IP 주소 출력 (ESP32 코드에 입력할 IP)
echo "============================="
echo " 라즈베리파이 IP 주소"
echo "============================="
hostname -I
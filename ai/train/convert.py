import tensorflow as tf
import numpy as np

# 모델 로드
model = tf.keras.models.load_model("gesture_model.h5")

# TFLite 변환
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

# .tflite 저장
with open("../model/gesture_model.tflite", "wb") as f:
    f.write(tflite_model)
print(f"✅ 모델 크기: {len(tflite_model)} bytes")

# C 배열로 변환 (ESP32에 넣을 형태)
# 터미널에서 실행:
# xxd -i ../model/gesture_model.tflite > ../model/gesture_model.h
print("✅ 다음 명령어 실행:")
print("xxd -i ../model/gesture_model.tflite > ../model/gesture_model.h")
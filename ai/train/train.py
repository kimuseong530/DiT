import numpy as np
import pandas as pd
import tensorflow as tf
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder

# 데이터 로드
df = pd.read_csv("gesture_data.csv",
                 names=["pitch","roll","ax","ay","az","gx","gy","gz","label"])

print(f"총 샘플 수: {len(df)}")
print(df["label"].value_counts())

X = df[["pitch","roll","ax","ay","az","gx","gy","gz"]].values
y = df["label"].values

# 데이터 분할
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

# 정규화
from sklearn.preprocessing import StandardScaler
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test  = scaler.transform(X_test)

# 스케일러 저장 (ESP32 추론 시 필요)
import json
scaler_params = {
    "mean": scaler.mean_.tolist(),
    "scale": scaler.scale_.tolist()
}
with open("scaler_params.json", "w") as f:
    json.dump(scaler_params, f)
print("✅ scaler_params.json 저장 완료")

# 모델 (ESP32용 최대한 작게!)
model = tf.keras.Sequential([
    tf.keras.layers.Dense(16, activation='relu', input_shape=(8,)),
    tf.keras.layers.Dense(16, activation='relu'),
    tf.keras.layers.Dense(5,  activation='softmax')
])

model.compile(
    optimizer='adam',
    loss='sparse_categorical_crossentropy',
    metrics=['accuracy']
)

model.summary()

# 학습
history = model.fit(
    X_train, y_train,
    epochs=100,
    batch_size=16,
    validation_data=(X_test, y_test)
)

# 정확도 출력
loss, acc = model.evaluate(X_test, y_test)
print(f"\n✅ 테스트 정확도: {acc*100:.1f}%")

# 모델 저장
model.save("gesture_model.h5")
print("✅ gesture_model.h5 저장 완료")
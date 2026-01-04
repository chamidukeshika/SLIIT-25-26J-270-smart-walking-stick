import cv2
from tensorflow.keras.models import load_model
import numpy as np
import pyttsx3

# -------------------- Load Models --------------------

model = load_model("facialemotionmodel.keras")

haar_file = cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
face_cascade = cv2.CascadeClassifier(haar_file)

genderProto = "gender_deploy.prototxt"
genderModel = "gender_net.caffemodel"
ageProto = "age_deploy.prototxt"
ageModel = "age_net.caffemodel"

genderNet = cv2.dnn.readNet(genderModel, genderProto)
ageNet = cv2.dnn.readNet(ageModel, ageProto)

# -------------------- Labels --------------------

genderList = ['Male', 'Female']
ageList = ['(0-2)', '(4-6)', '(8-12)', '(15-20)', '(25-32)', '(38-43)', '(48-53)', '(60-100)']
MODEL_MEAN_VALUES = (78.4263377603, 87.7689143744, 114.895847746)

emotion_labels = {
    0: 'angry',
    1: 'disgust',
    2: 'fear',
    3: 'happy',
    4: 'neutral',
    5: 'sad',
    6: 'surprise'
}

# -------------------- Functions --------------------

def extract_features(image):
    image = np.array(image).reshape(1, 48, 48, 1)
    return image / 255.0

# -------------------- TTS ENGINE --------------------

engine = pyttsx3.init()
engine.setProperty('rate', 150)
tts_busy = False   # 🔥 track engine state

# -------------------- Webcam --------------------

webcam = cv2.VideoCapture(0)


while True:
    ret, frame = webcam.read()
    if not ret:
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray, 1.3, 5)

    face_infos = []

    for (x, y, w, h) in faces:
        cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)

        # Emotion
        face_gray = gray[y:y + h, x:x + w]
        face_gray = cv2.resize(face_gray, (48, 48))
        emotion_pred = model.predict(extract_features(face_gray), verbose=0)
        emotion = emotion_labels[emotion_pred.argmax()]

        # Age & Gender
        face_color = frame[y:y + h, x:x + w]
        blob = cv2.dnn.blobFromImage(face_color, 1.0, (227, 227),
                                     MODEL_MEAN_VALUES, swapRB=False)

        genderNet.setInput(blob)
        gender = genderList[genderNet.forward()[0].argmax()]

        ageNet.setInput(blob)
        age = ageList[ageNet.forward()[0].argmax()]

        text = f"{emotion}, {gender}, {age}"
        face_infos.append(text)

        cv2.putText(frame, text, (x, y - 10),
                    cv2.FONT_HERSHEY_COMPLEX_SMALL, 1,
                    (0, 0, 255), 2)

    cv2.imshow("Output", frame)

    key = cv2.waitKey(1) & 0xFF

    # -------------------- KEY CONTROLS --------------------

    if key == ord('q'):
        break

    
    elif key == ord('s'):   # SPEAK
        for i, info in enumerate(face_infos):
            text = f"Person {i+1}: {info}"
            print(text)
            engine.say(text)
            engine.runAndWait()

            


engine.stop()
webcam.release()
cv2.destroyAllWindows()

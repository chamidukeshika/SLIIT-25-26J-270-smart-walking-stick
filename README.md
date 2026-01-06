# 🦯 SLIIT-25-26J-270-smart-walking-stick

An **intelligent, multi-modal smart walking stick** for the visually impaired that combines **emotional awareness** with **physical stability prediction**.  
Powered by **TinyML on ESP32**, this system not only navigates the environment but also **understands the user’s state** to prevent accidents *before they happen*.

---

## 🔎 Project Overview

This smart cane enhances **safe mobility** and **emotional well-being** by integrating:

- Environment perception  
- Stress prediction  
- Emotion detection  
- Emergency voice detection  

All processing is performed **on-device**, ensuring **low latency**, **privacy**, and **energy efficiency**.

---


## 🎯 Objectives

- ✅ Assist visually impaired users with **safe navigation**
- ✅ Predict and prevent **falls caused by gait instability**
- ✅ Detect **user stress**
- ✅ Recognize **environmental obstacles and landmarks**
- ✅ Enable **emergency detection and alerts**
- ✅ Operate efficiently on **low-power embedded hardware (ESP32)**

---

## 🧠 Core AI Modules

### 🧠 Emotion & Facial Awareness
- Detects **facial emotion**, **age**, and **gender**
- Provides contextual awareness for **social interaction cues**
- Identifies **stress or anxiety states**
- Implemented using **TinyML-optimized vision models**

---

### 🚷 Environment Perception  
*(Panel Recommendation Applied )*

> **Panel Suggestion:** Combine related components to reduce complexity and improve efficiency.

- 🔗 **Merged Obstacle Detection & Landmark Detection** into a single unified module
- Uses **camera + ultrasonic sensor fusion**
- Detects:
  - Static and dynamic obstacles
  - Landmarks (doors, stairs, poles, crossings)
  - Navigable gaps and safe paths
- 🚀 Improves accuracy while reducing processing overhead

---

### 🆘 Emergency Voice Detection
- Continuously listens for **vocal distress keywords**
- Automatically triggers **SMS alerts** to emergency contacts
- Operates **offline** for reliability
- Designed for **fast response** in critical situations

---

### 🚶 Gait Instability & Stress Prediction  

> **Newly introduced beyond the original proposal**

- Analyzes:
  - Motion patterns
  - Walking rhythm
  - Micro-imbalances
- ⚠️ Predicts **fall risk before it occurs**
- Detects **physical stress and fatigue**
- Provides **early warning feedback**
- Enhances **user safety and confidence**

---

## 🧩 System Architecture Overview

### 🔌 Sensors
- 📷 Camera  
- 📡 Ultrasonic Sensors  
- 🧭 IMU (Accelerometer & Gyroscope)  
- 🎤 Microphone  

### 🧠 Processing
- ESP32 with **TinyML models**
- Fully **on-device inference** (no cloud dependency)

### 🔊 Outputs
- Audio feedback
- Haptic alerts
- SMS emergency notifications

---

## 🛠️ Technologies Used

- **Hardware:** ESP32, Ultrasonic Sensors, IMU, Camera Module, Microphone  
- **AI / ML:** TinyML, TensorFlow Lite Micro  
- **Programming:** C/C++, Python (model training)  
- **Communication:** GSM / SMS  
- **Power:** Low-energy embedded system design  

---

## 🚀 Key Innovations

- ⭐ Combines **emotional intelligence** with **physical safety**
- ⭐ Predictive **fall-prevention** instead of reactive alerts
- ⭐ Efficient **edge-AI deployment** on constrained hardware
- ⭐ Panel-recommended **component optimization**
- ⭐ Added **stress & gait analysis module** for higher academic value

---

## 📈 Academic & Evaluation Highlights

- ✅ Panel feedback successfully implemented  
- ⭐ Additional AI module added for enhanced functionality  
- ✅ Improved modular design and system efficiency  
- ⭐ Strong contribution toward **extra evaluation marks**

---

## 👨‍💻 Team & Institution

- **Project Code:** SLIIT-25-26J-270  
- **Institution:** Sri Lanka Institute of Information Technology (SLIIT)  
- **Project Type:** Undergraduate Research / Final Year Project  

---

## 📄 License

This project is developed for **academic and research purposes** under **SLIIT guidelines**.

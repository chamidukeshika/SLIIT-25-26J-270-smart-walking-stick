# Obstacle and Landmarks Detection Component

## Overview

This component implements TinyML-based vision system for detecting obstacles and landmarks (crosswalks and traffic lights) using ESP32-CAM and Edge Impulse AI models. The system provides real-time object detection capabilities for the smart walking stick project.

## Features

- **On-Device AI Inference**: Runs Edge Impulse trained models directly on ESP32-CAM
- **Real-Time Detection**: Detects crosswalks and traffic lights in real-time
- **OLED Display Output**: Visual feedback on 0.96" OLED display (optional)
- **Serial Monitor Output**: Debug and detection information via serial communication
- **Offline Operation**: Completely offline, no internet required
- **Optimized Performance**: Smoothing and stability algorithms to reduce false positives

## Hardware Requirements

### Required Components
- **ESP32-CAM** (AI-Thinker module with OV2640 camera)
- **FTDI Programmer** or USB-to-Serial adapter for programming
- **Power Supply** (5V, minimum 500mA recommended)

### Optional Components
- **0.96" OLED Display** (128x64, I2C interface, SSD1306 controller)
- **Jumper Wires** for connections

## Hardware Connections

### ESP32-CAM Pinout
| Pin | Function | Description |
|-----|----------|-------------|
| 5V | Power | Supply voltage |
| GND | Ground | Ground reference |
| GPIO 14 | I2C SDA | OLED data line (optional) |
| GPIO 15 | I2C SCL | OLED clock line (optional) |

### OLED Display Connection (Optional)
| OLED Pin | ESP32-CAM Pin |
|----------|---------------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 14 |
| SCL | GPIO 15 |

## Software Requirements

### Arduino IDE Setup
1. **Install Arduino IDE** (version 1.8.x or 2.x)
2. **Install ESP32 Board Support**:
   - Add to Board Manager URLs: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Install "ESP32 by Espressif Systems"

### Required Libraries
Install via Arduino Library Manager:
- **esp_camera** (included with ESP32 board support)
- **Adafruit GFX Library** (for OLED, optional)
- **Adafruit SSD1306** (for OLED, optional)
- **Edge Impulse Arduino Library** (download from Edge Impulse project)

### Edge Impulse Model
1. Train your model on Edge Impulse platform
2. Download Arduino library from your Edge Impulse project
3. Install the library in Arduino IDE
4. Update the include statement in `tinyml_vision_final.ino`:
   ```cpp
   #include <YourProjectName_inferencing.h>
   ```

## File Structure

```
obstacle_and_landmarks/
├── config.h                        # Configuration settings
├── camera_handler.h                # Camera initialization and capture
├── display_handler.h               # OLED display functions
├── tinyml_vision_final.ino        # Main Arduino sketch
├── EI_ESP32_CAMERA_EXAMPLE_PATH.txt # Reference path
└── README.md                       # This file
```

## Configuration

All settings can be adjusted in [config.h](config.h):

### Camera Settings
- `CAMERA_FRAME_SIZE`: Frame size for inference (default: FRAMESIZE_96X96)
- `JPEG_QUALITY`: Image quality (0-63, default: 12)

### AI Model Settings
- `DETECTION_THRESHOLD`: Global confidence threshold (default: 0.60)
- `DETECTION_COOLDOWN`: Minimum time between detections in ms (default: 500)
- `INFERENCE_DELAY`: Delay between inference cycles in ms (default: 300)

### Display Settings
- `I2C_SDA`: SDA pin for I2C (default: GPIO 14)
- `I2C_SCL`: SCL pin for I2C (default: GPIO 15)
- `SCREEN_ADDRESS`: I2C address (default: 0x3C)

### Debug Settings
- `SERIAL_BAUD`: Serial communication speed (default: 115200)
- `DEBUG_MODE`: Enable verbose debugging (default: true)

## Usage

### 1. Upload Code
1. Open `tinyml_vision_final.ino` in Arduino IDE
2. Select board: **AI Thinker ESP32-CAM**
3. Connect FTDI programmer to ESP32-CAM
4. Put ESP32-CAM in programming mode (connect GPIO 0 to GND)
5. Upload the sketch
6. Remove GPIO 0 to GND connection and reset

### 2. Monitor Detection
- Open Serial Monitor (115200 baud)
- View detection results and debug information
- Optionally view results on OLED display

### 3. Adjust Settings
- Modify thresholds in [config.h](config.h) for better accuracy
- Tune per-label thresholds in main sketch:
  ```cpp
  static const float THRESH_CROSSWALK = 0.60f;
  static const float THRESH_TRAFFIC_LIGHT = 0.45f;
  ```

## Detection Classes

The system currently detects:
- **Crosswalk**: Pedestrian crossing markers
- **Traffic Light**: Traffic signal lights (red, yellow, green)

## Performance Optimization

### Smoothing Algorithm
- Implements exponential moving average for score smoothing
- Reduces detection flicker and false positives
- Configurable smoothing factor: `SMOOTH_ALPHA` (0.0 - 1.0)

### Stability Requirements
- Requires multiple consecutive stable frames before reporting detection
- Configurable via `STABLE_FRAMES_REQUIRED` (default: 3)

### Per-Label Thresholds
- Different confidence thresholds for each detection class
- Allows fine-tuning for specific detection requirements

## Troubleshooting

### Camera Issues
- **Camera init failed**: Check power supply (needs minimum 500mA)
- **Brownout detector**: Use external 5V power source, not USB
- **Image capture failed**: Reset ESP32-CAM and try again

### Detection Issues
- **Too many false positives**: Increase `DETECTION_THRESHOLD`
- **Missing detections**: Decrease threshold or retrain model with more data
- **Flickering detections**: Increase `STABLE_FRAMES_REQUIRED`

### OLED Display Issues
- **Display not working**: Check I2C connections and address (0x3C or 0x3D)
- **Garbled display**: Verify correct voltage (3.3V for most OLED modules)

## Integration with Smart Walking Stick

This component is designed to integrate with other components:
- **Audio Detection**: Combine with audio feedback for user alerts
- **Emotion Detection**: Context-aware assistance based on user state
- **Pressure/Stress Detection**: Adaptive detection based on user condition

### Communication Interface
- Serial communication for inter-component messaging
- Can be extended with I2C, SPI, or other protocols
- JSON or binary protocol for data exchange

## Future Enhancements

- [ ] Add more detection classes (vehicles, pedestrians, etc.)
- [ ] Implement depth estimation for distance measurement
- [ ] Add day/night mode with automatic adjustment
- [ ] Optimize power consumption for battery operation
- [ ] Add wireless communication (WiFi/BLE) for data logging
- [ ] Implement multi-object tracking

## Development Notes

### Model Training Tips
1. Collect diverse training data (different lighting, angles, distances)
2. Balance dataset across all classes
3. Use data augmentation in Edge Impulse
4. Test model thoroughly before deployment
5. Consider quantization for better performance

### Code Optimization
- Uses PSRAM for frame buffers when available
- Efficient RGB888 conversion and resizing
- Minimal memory allocations in main loop
- Optimized for real-time performance

## License

This component is part of the SLIIT-25-26J-270-smart-walking-stick research project.

## Contributors

- Research Team: SLIIT 2025-2026 Batch
- Component: Obstacle and Landmarks Detection

## References

- [Edge Impulse Documentation](https://docs.edgeimpulse.com/)
- [ESP32-CAM Guide](https://github.com/espressif/esp32-camera)
- [TinyML Book](https://www.oreilly.com/library/view/tinyml/9781492052036/)

## Support

For issues or questions:
1. Check troubleshooting section above
2. Review Edge Impulse example path in `EI_ESP32_CAMERA_EXAMPLE_PATH.txt`
3. Consult project documentation
4. Contact research team

---

**Last Updated**: January 2026  
**Version**: 1.0  
**Status**: Active Development

/*
 * Display Handler
 * 
 * Functions to control the OLED display and show information
 */

#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include "config.h"

// ============================================
// DISPLAY INITIALIZATION
// ============================================

bool initDisplay(Adafruit_SSD1306 *display) {
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);
  
  // Initialize OLED
  if (!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED initialization failed");
    Serial.println("Check wiring and I2C address");
    return false;
  }
  
  // Clear display
  display->clearDisplay();
  display->display();
  
  return true;
}

// ============================================
// SHOW STARTUP MESSAGE
// ============================================

void showStartup(Adafruit_SSD1306 *display) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  
  // Title (big)
  display->setTextSize(2);
  display->setCursor(10, 10);
  display->println("TinyML");
  display->setCursor(10, 30);
  display->println("Vision");
  
  // Status (small)
  display->setTextSize(1);
  display->setCursor(0, 55);
  display->println("Initializing...");
  
  display->display();
}

// ============================================
// SHOW READY STATUS
// ============================================

void showReady(Adafruit_SSD1306 *display) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  
  // Title
  display->setTextSize(1);
  display->setCursor(0, 5);
  display->println(SYSTEM_NAME);
  
  // Status (big)
  display->setTextSize(2);
  display->setCursor(25, 25);
  display->println("READY");
  
  // Info
  display->setTextSize(1);
  display->setCursor(20, 55);
  display->println("Waiting...");
  
  display->display();
}

// ============================================
// SHOW DETECTION
// ============================================

void showDetection(Adafruit_SSD1306 *display, String label, int confidence) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  
  // Title
  display->setTextSize(1);
  display->setCursor(0, 0);
  display->println("DETECTED:");
  
  // Draw line
  display->drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
  
  // Convert Edge Impulse label to display name
  String displayLabel = label;
  displayLabel.toUpperCase();
  
  // Replace underscores with spaces
  displayLabel.replace("_", " ");
  
  // Object name (big, wrap if needed)
  display->setTextSize(2);
  
  // Check label length and adjust display
  if (displayLabel.length() <= 6) {
    // Short label - display on one line, centered
    display->setCursor(5, 22);
    display->println(displayLabel);
  } else if (displayLabel.length() <= 12) {
    // Medium label - display on one line, small
    display->setCursor(0, 22);
    display->println(displayLabel);
  } else {
    // Long label - split into two lines
    int spacePos = displayLabel.indexOf(' ');
    if (spacePos > 0) {
      String line1 = displayLabel.substring(0, spacePos);
      String line2 = displayLabel.substring(spacePos + 1);
      display->setCursor(0, 15);
      display->println(line1);
      display->setCursor(0, 33);
      display->println(line2);
    } else {
      // No space, just show first part
      display->setCursor(0, 22);
      display->println(displayLabel.substring(0, 12));
    }
  }
  
  // Confidence (small, at bottom)
  display->setTextSize(1);
  display->setCursor(0, 55);
  display->print("Confidence: ");
  display->print(confidence);
  display->println("%");
  
  display->display();
}

// ============================================
// SHOW GENERIC MESSAGE
// ============================================

void showMessage(Adafruit_SSD1306 *display, String line1, String line2, String line3) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  
  // Line 1 (small, top)
  display->setTextSize(1);
  display->setCursor(0, 5);
  display->println(line1);
  
  // Line 2 (big, middle)
  display->setTextSize(2);
  display->setCursor(0, 25);
  display->println(line2);
  
  // Line 3 (small, bottom)
  display->setTextSize(1);
  display->setCursor(0, 55);
  display->println(line3);
  
  display->display();
}

// ============================================
// SHOW ERROR
// ============================================

void showError(Adafruit_SSD1306 *display, String errorMsg) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  
  // Title
  display->setTextSize(1);
  display->setCursor(0, 0);
  display->println("ERROR!");
  
  // Draw line
  display->drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
  
  // Error message
  display->setTextSize(1);
  display->setCursor(0, 20);
  
  // Word wrap error message
  int lineY = 20;
  int startPos = 0;
  while (startPos < errorMsg.length() && lineY < SCREEN_HEIGHT - 10) {
    int endPos = startPos + 21;  // Max chars per line
    if (endPos >= errorMsg.length()) {
      endPos = errorMsg.length();
    } else {
      // Try to break at space
      int spacePos = errorMsg.lastIndexOf(' ', endPos);
      if (spacePos > startPos) {
        endPos = spacePos;
      }
    }
    
    String line = errorMsg.substring(startPos, endPos);
    display->setCursor(0, lineY);
    display->println(line);
    
    startPos = endPos + 1;
    lineY += 10;
  }
  
  display->display();
}

// ============================================
// CLEAR DISPLAY
// ============================================

void clearDisplay(Adafruit_SSD1306 *display) {
  display->clearDisplay();
  display->display();
}

// ============================================
// SHOW NO DETECTION
// ============================================

void showNoDetection(Adafruit_SSD1306 *display) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  
  // Title
  display->setTextSize(1);
  display->setCursor(0, 5);
  display->println(SYSTEM_NAME);
  
  // Status
  display->setTextSize(2);
  display->setCursor(35, 25);
  display->println("NO");
  display->setCursor(0, 45);
  display->println("DETECTION");
  
  display->display();
}

// ============================================
// SHOW SCANNING
// ============================================

void showScanning(Adafruit_SSD1306 *display) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  
  display->setTextSize(2);
  display->setCursor(5, 20);
  display->println("Scanning");
  
  display->setTextSize(1);
  display->setCursor(35, 45);
  display->println("...");
  
  display->display();
}

// ============================================
// SHOW BATTERY/STATUS (for future use)
// ============================================

void showStatus(Adafruit_SSD1306 *display, String status) {
  // Small status indicator in corner
  display->setTextSize(1);
  display->setCursor(SCREEN_WIDTH - 30, 0);
  display->println(status);
  display->display();
}

#endif // DISPLAY_HANDLER_H

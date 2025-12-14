#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "config.h"
#include "led.h"
#include "camera.h"
#include "api.h"
#include "webserver.h"

// ===== Capture Flow =====
void captureAndUpload() {
  Serial.println("\n========================================");
  Serial.println("[MAIN] Starting capture...");
  Serial.println("========================================");

  Led::working();

  // Pause stream before capture
  WebServer::pauseStream();
  delay(100);

  // Step 1: Capture
  Serial.println("[1/4] Capturing photo...");
  camera_fb_t* fb = Camera::capture();
  if (!fb) {
    Serial.println("[ERROR] Capture failed!");
    Led::error();
    WebServer::resumeStream();
    return;
  }
  Serial.printf("[OK] Size: %d bytes\n", fb->len);

  // Step 2: Get presigned URL
  Serial.println("[2/4] Getting upload URL...");
  UploadUrls urls = Api::getUploadUrl(DEVICE_ID);
  if (!urls.success) {
    Serial.println("[ERROR] Failed to get URL!");
    Camera::release(fb);
    Led::error();
    WebServer::resumeStream();
    return;
  }

  // Step 3: Upload to R2
  Serial.println("[3/4] Uploading to R2...");
  if (!Api::uploadImage(urls.uploadUrl, fb)) {
    Serial.println("[ERROR] Upload failed!");
    Camera::release(fb);
    Led::error();
    WebServer::resumeStream();
    return;
  }
  Camera::release(fb);

  // Step 4: Save record & get classification
  Serial.println("[4/4] Saving record & classifying...");
  TrashRecord record;
  record.deviceId = DEVICE_ID;
  record.imageUrl = urls.imageUrl;
  record.latitude = 13.756331;   // Mock GPS
  record.longitude = 100.501762;

  ClassificationResult result = Api::saveTrashRecord(record);

  if (!result.success) {
    Serial.println("[ERROR] Save/classify failed!");
    if (result.error.length() > 0) {
      Serial.println("[ERROR] " + result.error);
    }
    Led::error();
    WebServer::resumeStream();
    return;
  }

  // Display classification result
  Serial.println("========================================");
  Serial.println("[SUCCESS] Classification Complete!");
  Serial.println("========================================");
  Serial.println("  Category:   " + result.category);
  if (result.subCategory.length() > 0) {
    Serial.println("  SubCategory: " + result.subCategory);
  }
  Serial.printf("  Confidence: %.1f%%\n", result.confidence * 100);
  Serial.printf("  Bin Number: %d\n", result.binNumber);
  Serial.println("  Bin Label:  " + result.binLabel);
  if (result.message.length() > 0) {
    Serial.println("  Message:    " + result.message);
  }
  Serial.println("========================================\n");

  // LED feedback: blink bin number
  Led::success();
  Led::showBinNumber(result.binNumber);
  Led::idle();

  // Resume stream
  WebServer::resumeStream();
}

// ===== Setup =====
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(1000);

  Serial.println("\n");
  Serial.println("============================================");
  Serial.println("    Smart Trash Picker - ESP32-CAM");
  Serial.println("============================================\n");

  // Init LED
  Led::init();

  // Init Camera
  Serial.println("[INIT] Camera...");
  if (!Camera::init()) {
    Serial.println("[FATAL] Camera failed!");
    while (1) { Led::error(); delay(1000); }
  }

  // Connect WiFi
  Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > WIFI_TIMEOUT_MS) {
      Serial.println("\n[FATAL] WiFi timeout!");
      while (1) { Led::error(); delay(1000); }
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("[OK] WiFi connected!");
  Serial.print("     IP: ");
  Serial.println(WiFi.localIP());

  // Init API (for HTTPS)
  Api::init();

  // Start Web Server
  WebServer::init();

  Serial.println();
  Serial.println("============================================");
  Serial.println("  READY!");
  Serial.println("  1. Type 'c' + Enter in Serial Monitor");
  Serial.print("  2. Open http://");
  Serial.println(WiFi.localIP());
  Serial.println("============================================\n");

  Led::idle();
}

// ===== Loop =====
void loop() {
  // Serial command
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'c' || c == 'C') {
      Serial.println("[SERIAL] Capture!");
      captureAndUpload();
    }
  }

  // Web command
  if (WebServer::isCaptureRequested()) {
    WebServer::clearCaptureRequest();
    captureAndUpload();
  }

  delay(10);
}

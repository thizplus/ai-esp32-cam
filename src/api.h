#ifndef API_H
#define API_H

#include <Arduino.h>
#include "esp_camera.h"

struct UploadUrls {
  String uploadUrl;
  String imageUrl;
  bool success;
};

struct TrashRecord {
  String deviceId;
  String imageUrl;
  float latitude;
  float longitude;
};

// Classification result from AI service
struct ClassificationResult {
  bool success;
  String category;
  String subCategory;
  float confidence;
  int binNumber;
  String binLabel;
  String message;
  String error;
  // L0 (YOLO) detection info
  String l0Label;       // YOLO detected object (bottle, cup, etc.)
  float l0Confidence;   // YOLO confidence
};

namespace Api {
  void init();
  UploadUrls getUploadUrl(const char* deviceId);
  bool uploadImage(const String& uploadUrl, camera_fb_t* fb);
  ClassificationResult saveTrashRecord(const TrashRecord& record);
}

#endif

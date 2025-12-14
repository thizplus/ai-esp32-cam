#include "api.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

namespace Api {

WiFiClientSecure httpsClient;

void init() {
  WiFi.setSleep(false);
  httpsClient.setInsecure();  // Skip cert verification for R2
  Serial.println("[API] Initialized");
}

// Build API URL
String buildUrl(const char* path) {
  String url = "http://";
  url += API_HOST;
  url += ":";
  url += API_PORT;
  url += path;
  return url;
}

UploadUrls getUploadUrl(const char* deviceId) {
  UploadUrls result = {"", "", false};

  HTTPClient http;
  String url = buildUrl("/api/upload-url?device_id=") + deviceId;

  Serial.println("[API] GET " + url);

  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT_MS);

  int httpCode = http.GET();

  if (httpCode != 200) {
    Serial.printf("[API] Failed. Code: %d\n", httpCode);
    http.end();
    return result;
  }

  String payload = http.getString();
  http.end();

  Serial.println("[API] Response: " + payload);

  // Parse JSON with ArduinoJson
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.printf("[API] JSON parse error: %s\n", error.c_str());
    return result;
  }

  if (doc["success"] == true) {
    result.uploadUrl = doc["data"]["upload_url"].as<String>();
    result.imageUrl = doc["data"]["image_url"].as<String>();
    result.success = true;

    Serial.println("[API] upload_url: " + result.uploadUrl.substring(0, 60) + "...");
    Serial.println("[API] image_url: " + result.imageUrl);
  }

  return result;
}

bool uploadImage(const String& uploadUrl, camera_fb_t* fb) {
  if (!fb || fb->len == 0) {
    Serial.println("[API] Invalid frame buffer");
    return false;
  }

  Serial.printf("[API] PUT image (%d bytes)\n", fb->len);

  HTTPClient http;
  http.begin(httpsClient, uploadUrl);
  http.addHeader("Content-Type", "image/jpeg");
  http.setTimeout(HTTP_TIMEOUT_MS);

  int httpCode = http.sendRequest("PUT", fb->buf, fb->len);
  http.end();

  Serial.printf("[API] Response code: %d\n", httpCode);

  if (httpCode == 200 || httpCode == 201) {
    Serial.println("[API] Image uploaded!");
    return true;
  }

  Serial.println("[API] Upload failed!");
  return false;
}

ClassificationResult saveTrashRecord(const TrashRecord& record) {
  ClassificationResult result = {false, "", "", 0.0, 0, "", "", ""};

  HTTPClient http;
  String url = buildUrl("/api/trash");

  Serial.println("[API] POST " + url);

  // Build JSON with ArduinoJson
  JsonDocument reqDoc;
  reqDoc["device_id"] = record.deviceId;
  reqDoc["image_url"] = record.imageUrl;
  reqDoc["latitude"] = record.latitude;
  reqDoc["longitude"] = record.longitude;

  String json;
  serializeJson(reqDoc, json);

  Serial.println("[API] Body: " + json);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(HTTP_TIMEOUT_MS);

  int httpCode = http.POST(json);
  String response = http.getString();
  http.end();

  Serial.printf("[API] Response code: %d\n", httpCode);
  Serial.println("[API] Response: " + response);

  if (httpCode != 200 && httpCode != 201) {
    Serial.println("[API] Save failed!");
    result.error = "HTTP Error: " + String(httpCode);
    return result;
  }

  // Parse response to get classification result
  JsonDocument respDoc;
  DeserializationError error = deserializeJson(respDoc, response);

  if (error) {
    Serial.printf("[API] JSON parse error: %s\n", error.c_str());
    result.error = "JSON parse error";
    return result;
  }

  if (respDoc["success"] == true) {
    JsonObject data = respDoc["data"];
    result.success = true;
    result.category = data["category"].as<String>();
    result.subCategory = data["sub_category"].as<String>();
    result.confidence = data["confidence"].as<float>();
    result.binNumber = data["bin_number"].as<int>();
    result.binLabel = data["bin_label"].as<String>();
    result.message = data["message"].as<String>();
    result.error = data["classify_error"].as<String>();

    Serial.println("[API] Record saved with classification!");
  } else {
    result.error = respDoc["message"].as<String>();
    Serial.println("[API] Save failed: " + result.error);
  }

  return result;
}

}

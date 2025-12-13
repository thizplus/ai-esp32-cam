#include <Arduino.h>
#include "esp_camera.h"
#include "WiFi.h"
#include "esp_http_server.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ===== ใส่ชื่อและรหัส WiFi ตรงนี้ =====
const char* ssid = "IDEAKRUSH 2.4GHz";
const char* password = "1234567890";
// =====================================

// ขา GPIO สำหรับ ESP32-CAM (AI-Thinker)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }
  Serial.println("Camera init success!");
  return true;
}

// หน้าเว็บหลัก
static esp_err_t index_handler(httpd_req_t *req) {
  const char* html = R"(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM</title>
  <style>
    body { font-family: Arial; text-align: center; background: #1a1a1a; color: white; }
    h1 { margin-top: 20px; }
    img { max-width: 100%; border: 2px solid #444; border-radius: 10px; }
  </style>
</head>
<body>
  <h1>ESP32-CAM Live Stream</h1>
  <img src="/stream">
</body>
</html>
)";
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, html, strlen(html));
}

// Live stream handler
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  char part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      size_t hlen = snprintf(part_buf, 64, _STREAM_PART, fb->len);
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
      if (res == ESP_OK) res = httpd_resp_send_chunk(req, part_buf, hlen);
      if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
      esp_camera_fb_return(fb);
    }
    if (res != ESP_OK) break;
  }
  return res;
}

void startWebServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler };
    httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler };
    httpd_register_uri_handler(stream_httpd, &index_uri);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println("Web server started!");
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(1000);
  Serial.println("===================");
  Serial.println("ESP32-CAM Starting...");
  Serial.println("===================");

  // เริ่มต้นกล้อง
  if (!initCamera()) {
    Serial.println("Camera failed!");
    return;
  }

  // เชื่อมต่อ WiFi
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // เริ่ม Web Server
  startWebServer();

  Serial.println("===================");
  Serial.println("Open browser and go to:");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("===================");
}

void loop() {
  delay(1000);
}
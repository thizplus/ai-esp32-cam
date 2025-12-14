#ifndef CONFIG_H
#define CONFIG_H

// ===== WiFi =====
#define WIFI_SSID "IDEAKRUSH 2.4GHz"
#define WIFI_PASSWORD "1234567890"
#define WIFI_TIMEOUT_MS 15000

// ===== API =====
// ใช้ IP ของคอมพิวเตอร์ใน LAN (หาจาก ipconfig)
#define API_HOST "192.168.1.24"
#define API_PORT 8080
#define API_USE_HTTPS false       // false = HTTP, true = HTTPS

#define DEVICE_ID "ESP32CAM001"
#define HTTP_TIMEOUT_MS 30000

// ===== Hardware Pins =====
// Camera (AI-Thinker ESP32-CAM)
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

// LED
#define LED_FLASH 4
#define LED_STATUS 33

// ===== Camera Settings =====
#define CAM_FRAME_SIZE FRAMESIZE_VGA
#define CAM_JPEG_QUALITY 12

#endif

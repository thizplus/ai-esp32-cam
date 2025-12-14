#include "webserver.h"
#include "config.h"
#include "esp_http_server.h"
#include "esp_camera.h"

namespace WebServer {

static httpd_handle_t server = NULL;
static volatile bool captureRequested = false;

// HTML with auto-refresh preview
static const char* indexHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Trash Picker</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #1a1a2e, #16213e);
      color: white;
      min-height: 100vh;
      padding: 20px;
    }
    .container { max-width: 640px; margin: 0 auto; text-align: center; }
    h1 { font-size: 1.5em; margin-bottom: 5px; }
    .subtitle { color: #888; margin-bottom: 15px; font-size: 0.9em; }
    .preview {
      background: #000;
      border-radius: 10px;
      overflow: hidden;
      margin-bottom: 15px;
    }
    .preview img { width: 100%; display: block; }
    .btn {
      background: linear-gradient(135deg, #ff6b6b, #ee5a5a);
      color: white;
      border: none;
      padding: 18px 40px;
      font-size: 1.2em;
      border-radius: 10px;
      cursor: pointer;
      transition: all 0.3s;
    }
    .btn:hover { transform: translateY(-2px); box-shadow: 0 5px 20px rgba(255,107,107,0.4); }
    .btn:disabled { background: #555; cursor: not-allowed; transform: none; }
    .status {
      margin-top: 15px;
      padding: 12px;
      border-radius: 8px;
      background: rgba(255,255,255,0.1);
    }
    .status.success { background: rgba(0,212,170,0.2); }
    .status.error { background: rgba(255,107,107,0.2); }
  </style>
</head>
<body>
  <div class="container">
    <h1>Smart Trash Picker</h1>
    <p class="subtitle">ESP32-CAM</p>
    <div class="preview">
      <img id="preview" src="/preview" alt="Preview">
    </div>
    <button class="btn" id="btn" onclick="capture()">Capture & Upload</button>
    <div class="status" id="status">Ready</div>
  </div>
  <script>
    // Auto refresh preview
    setInterval(function() {
      var img = document.getElementById('preview');
      img.src = '/preview?' + Date.now();
    }, 500);

    function capture() {
      var btn = document.getElementById('btn');
      var status = document.getElementById('status');
      btn.disabled = true;
      btn.textContent = 'Uploading...';
      status.textContent = 'Taking photo and uploading...';
      status.className = 'status';

      fetch('/capture')
        .then(function(r) { return r.text(); })
        .then(function(data) {
          status.textContent = data;
          status.className = 'status success';
          btn.disabled = false;
          btn.textContent = 'Capture & Upload';
        })
        .catch(function(err) {
          status.textContent = 'Error: ' + err;
          status.className = 'status error';
          btn.disabled = false;
          btn.textContent = 'Capture & Upload';
        });
    }
  </script>
</body>
</html>
)rawliteral";

// Index handler
static esp_err_t indexHandler(httpd_req_t* req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, indexHtml, strlen(indexHtml));
}

// Preview handler - single image
static esp_err_t previewHandler(httpd_req_t* req) {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
  esp_err_t res = httpd_resp_send(req, (const char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return res;
}

// Capture handler
static esp_err_t captureHandler(httpd_req_t* req) {
  Serial.println("[WEB] Capture requested");
  captureRequested = true;

  // Wait for capture to complete (max 30 sec)
  int timeout = 300;
  while (captureRequested && timeout > 0) {
    delay(100);
    timeout--;
  }

  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, "Done! Check Serial Monitor for details.", -1);
}

void init() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t indexUri = { .uri = "/", .method = HTTP_GET, .handler = indexHandler, .user_ctx = NULL };
    httpd_uri_t previewUri = { .uri = "/preview", .method = HTTP_GET, .handler = previewHandler, .user_ctx = NULL };
    httpd_uri_t captureUri = { .uri = "/capture", .method = HTTP_GET, .handler = captureHandler, .user_ctx = NULL };

    httpd_register_uri_handler(server, &indexUri);
    httpd_register_uri_handler(server, &previewUri);
    httpd_register_uri_handler(server, &captureUri);

    Serial.println("[WEB] Server started on port 80");
  }
}

bool isCaptureRequested() {
  return captureRequested;
}

void clearCaptureRequest() {
  captureRequested = false;
}

void pauseStream() {}
void resumeStream() {}

}

# Smart Trash Picker - ESP32-CAM

ไม้คีบขยะอัจฉริยะที่ถ่ายภาพขยะอัตโนมัติ พร้อมบันทึกพิกัด GPS และอัพโหลดขึ้น Cloud

## สถานะโปรเจค

| ส่วน | สถานะ |
|------|--------|
| ESP32-CAM Firmware | Done |
| Web Preview + Capture | Done |
| Backend API (Go Fiber) | Done |
| Cloudflare R2 Upload | Done |
| Database (PostgreSQL) | Done |
| AI Classification | Pending |
| GPS Integration | Pending |

---

## System Architecture

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   ESP32-CAM     │     │   Go Fiber      │     │  Cloudflare R2  │
│                 │     │   Backend       │     │                 │
│  - Camera       │────▶│  - /api/upload  │────▶│  - Image Store  │
│  - Web Server   │     │  - /api/trash   │     │                 │
│  - WiFi         │     │  - PostgreSQL   │     │                 │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

---

## Flow การทำงาน

```
1. เปิดหน้าเว็บ (http://<ESP32-IP>)
   │
2. เห็น Preview จากกล้อง (refresh ทุก 500ms)
   │
3. กดปุ่ม "Capture & Upload"
   │
4. ESP32 ถ่ายภาพ
   │
5. GET /api/upload-url ──▶ Backend สร้าง Presigned URL
   │
6. PUT image ──▶ Upload ไป Cloudflare R2
   │
7. POST /api/trash ──▶ บันทึกข้อมูลลง Database
   │
8. แสดงผลสำเร็จ
```

---

## โครงสร้างไฟล์ ESP32

```
smart-trash-picker/
├── platformio.ini          # PlatformIO config
├── README.md               # ไฟล์นี้
└── src/
    ├── main.cpp            # Entry point + capture flow
    ├── config.h            # Configuration (WiFi, API, Pins)
    ├── camera.h/.cpp       # Camera functions
    ├── led.h/.cpp          # LED feedback
    ├── api.h/.cpp          # HTTP API calls
    └── webserver.h/.cpp    # Web UI + preview
```

---

## Configuration

แก้ไขที่ `src/config.h`:

```cpp
// WiFi
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Backend API
#define API_HOST "192.168.1.30"    // IP ของเครื่องที่รัน Backend
#define API_PORT 8080
#define API_USE_HTTPS false

// Device
#define DEVICE_ID "ESP32CAM001"
```

---

## API Endpoints

### 1. GET /api/upload-url

ขอ Presigned URL สำหรับ upload รูป

**Request:**
```
GET /api/upload-url?device_id=ESP32CAM001
```

**Response:**
```json
{
  "success": true,
  "data": {
    "upload_url": "https://....r2.cloudflarestorage.com/...",
    "image_url": "https://pub-xxx.r2.dev/trash/.../xxx.jpg",
    "expires_in": 900
  }
}
```

### 2. PUT (Presigned URL)

Upload รูปไป Cloudflare R2

**Request:**
```
PUT {upload_url}
Content-Type: image/jpeg
Body: <binary image>
```

**Response:** `200 OK`

### 3. POST /api/trash

บันทึกข้อมูลขยะ

**Request:**
```json
{
  "device_id": "ESP32CAM001",
  "image_url": "https://pub-xxx.r2.dev/trash/.../xxx.jpg",
  "latitude": 13.756331,
  "longitude": 100.501762
}
```

**Response:**
```json
{
  "success": true,
  "data": {
    "id": "uuid",
    "device_id": "ESP32CAM001",
    "image_url": "...",
    "latitude": 13.756331,
    "longitude": 100.501762,
    "created_at": "2025-12-13T20:36:12Z"
  }
}
```

---

## วิธีใช้งาน

### 1. รัน Backend

```bash
cd gofiber-smart-trash
go run main.go
```

### 2. Upload Firmware

1. แก้ไข `src/config.h` ให้ตรงกับ WiFi และ IP ของ Backend
2. ต่อ **IO0 → GND** บน ESP32-CAM
3. กด **Reset**
4. กด **Upload** ใน PlatformIO
5. ถอด IO0-GND แล้วกด **Reset**

### 3. ใช้งาน

1. เปิด **Serial Monitor** (115200) ดู IP
2. เปิด Browser ไปที่ `http://<ESP32-IP>`
3. เห็น Preview → กดปุ่ม **Capture & Upload**

---

## Web Interface

```
┌────────────────────────────────┐
│     Smart Trash Picker         │
│         ESP32-CAM              │
│                                │
│  ┌──────────────────────────┐  │
│  │                          │  │
│  │       [Preview]          │  │
│  │    (refresh 500ms)       │  │
│  │                          │  │
│  └──────────────────────────┘  │
│                                │
│     [ Capture & Upload ]       │
│                                │
│          Ready                 │
└────────────────────────────────┘
```

---

## Libraries ที่ใช้

| Library | Version | Description |
|---------|---------|-------------|
| espressif32 | latest | ESP32 Platform |
| ArduinoJson | ^7.0.0 | JSON parsing |
| esp_camera | built-in | Camera driver |
| WiFi | built-in | WiFi connectivity |
| HTTPClient | built-in | HTTP requests |

---

## Hardware

| Component | GPIO | Description |
|-----------|------|-------------|
| Camera | Built-in | OV2640 (AI-Thinker) |
| LED Status | 33 | สถานะการทำงาน |
| LED Flash | 4 | แฟลชกล้อง |
| GPS (future) | 16, 17 | Neo-6M |
| Switch (future) | 12 | Micro switch |

---

## LED Feedback

| Pattern | Meaning |
|---------|---------|
| กระพริบช้า 2 ครั้ง | สำเร็จ |
| กระพริบเร็ว 5 ครั้ง | Error |
| ติดค้าง | กำลังทำงาน |

---

## Next Steps

### 1. AI Classification (ถัดไป)

เพิ่ม AI วิเคราะห์ประเภทขยะ:
- organic (เศษอาหาร, ใบไม้)
- recyclable (พลาสติก, กระดาษ, โลหะ, แก้ว)
- hazardous (แบตเตอรี่, สารเคมี)
- general (ขยะทั่วไป)

**ทางเลือก:**
- OpenAI Vision (GPT-4o)
- Google Gemini Vision
- Hugging Face Models

### 2. GPS Integration

เพิ่ม GPS Neo-6M สำหรับบันทึกพิกัดจริง

### 3. Physical Switch

เพิ่ม Micro Switch สำหรับกดถ่ายภาพตอนคีบขยะ

---

## Troubleshooting

### Upload ไม่ได้

1. ตรวจสอบ **IO0 → GND**
2. กด **Reset** ตอนเห็น `Connecting...`
3. ลองเปลี่ยนสาย USB

### WiFi ไม่เชื่อมต่อ

1. ตรวจสอบ SSID/Password ใน `config.h`
2. ใช้ WiFi 2.4GHz เท่านั้น (ESP32 ไม่รองรับ 5GHz)

### API Error

1. ตรวจสอบว่า Backend รันอยู่
2. ตรวจสอบ IP ใน `config.h`
3. ตรวจสอบว่า Backend bind `0.0.0.0` (ไม่ใช่ `127.0.0.1`)

---

## License

MIT

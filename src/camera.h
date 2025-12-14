#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include "esp_camera.h"
#include "config.h"

namespace Camera {
  bool init();
  camera_fb_t* capture();
  void release(camera_fb_t* fb);
}

#endif

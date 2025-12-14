#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>

namespace WebServer {
  void init();
  bool isCaptureRequested();
  void clearCaptureRequest();
  void pauseStream();
  void resumeStream();
}

#endif

#include <Arduino.h>
#include <time.h>
#include <Adafruit_LIS3DH.h>
#include <Notecard.h>
#include <accel-testing_inferencing.h>

#define productUID "com.blues.tvantoll:mlops"
#define serialDebug Serial

Notecard notecard;
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

void setup(void)
{
  serialDebug.begin(115200);
  while (!serialDebug) {}
  notecard.setDebugOutputStream(serialDebug);

  lis.begin(0x18);
  notecard.begin();

  J *req = notecard.newRequest("hub.set");
  JAddStringToObject(req, "product", productUID);
  JAddStringToObject(req, "mode", "continuous");
  if (!notecard.sendRequest(req)) {
      JDelete(req);
  }
}

void loop()
{
  #define EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE_COBS (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE + 256)
  int16_t buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE_COBS] = {0};

  for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3)
  {
    // Determine the next tick (and then sleep later)
    uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);

    lis.read();
    buffer[ix] = lis.x;
    buffer[ix + 1] = lis.y;
    buffer[ix + 2] = lis.z;

    delayMicroseconds(next_tick - micros());
  }

  // send binary data to the Notecard
  NoteBinaryTransmit(reinterpret_cast<uint8_t *>(buffer),
    (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE * 2),
    (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE_COBS * 2),
    false);

  // char buff[25] = "TJ VanToll";
  // NoteBinaryTransmit((uint8_t *) buff, 10, sizeof(buff), false);

  J *req = NoteNewRequest("web.post");
  JAddStringToObject(req, "route", "whatever");
  JAddBoolToObject(req, "binary", true);
  JAddBoolToObject(req, "live", true);
  NoteRequest(req);

  delay(30 * 1000);
}

#include <Arduino.h>
#include <ESP_AT_Lib.h>
#include <FastLED.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <AccelStepper.h>

HardwareSerial EspSerial(PB7, PB6);

#define SSID        "Obi LAN Kenobi"
#define PASSWORD    "IHaveTheHighGround"

#define HOST_NAME   "10.10.42.193"
#define HOST_PORT   (8000)

ESP8266 wifi(&EspSerial);

// Your board <-> ESP_AT baud rate:
#define ESP_AT_BAUD       115200

#define NUM_LEDS 10
#define DATA_PIN PA15

// manually define Pin mapping for Pin A15
_FL_DEFPIN(PA15, 15, A);

CRGB leds[NUM_LEDS];

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

const int stepsPerRevolution = 2048; // maybe x2 when using half-steps
AccelStepper motorRight(AccelStepper::FULL4WIRE, PA1, PA3, PA0, PA2);
AccelStepper motorLeft(AccelStepper::FULL4WIRE, PA4, PA6, PA5, PA7);

const float WHEEL_RADIUS = 0.0197; // unit: m
const float WHEEL_BASE = 0.0945; // unit: m
float rpmMax = 22;

float distanceToSteps(float distance) {
  float rotations = distance / (WHEEL_RADIUS * 2 * M_PI);
  return stepsPerRevolution * rotations;
}

void driveStraight(float distance) {
  float steps = distanceToSteps(distance);
  motorLeft.move(steps);
  motorRight.move(steps);
}

bool isMoving() {
  return motorLeft.isRunning() && motorRight.isRunning();
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);

  motorLeft.setMaxSpeed(stepsPerRevolution);
  motorRight.setMaxSpeed(stepsPerRevolution);
  motorLeft.setAcceleration(distanceToSteps(0.01)); // does this makes sense?
  motorRight.setAcceleration(distanceToSteps(0.01)); // does this makes sense?
  motorLeft.setSpeed(20);
  motorRight.setSpeed(20);

  // initialize serial for ESP module
  EspSerial.begin(ESP_AT_BAUD);

  wifi.setOprToStation();
  wifi.joinAP(SSID, PASSWORD);
  wifi.enableMUX();

  uint8_t buffer[128] = {0};

  static uint8_t mux_id = 0;

  wifi.createTCP(mux_id, HOST_NAME, HOST_PORT);

  char hello[] = "Hello, this is client!";

  wifi.send(mux_id, (const uint8_t*)hello, strlen(hello));

  uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 10000);

  if (len > 0)
  {
    for (uint32_t i = 0; i < len; i++)
    {
      buffer[i];
    }
  }

  wifi.releaseTCP(mux_id);

  // set the hw i2c pins for u8g2 to PB8 and PB9, because the default is PB6 and PB7
  Wire.setSDA(PB9);
  Wire.setSCL(PB8);
  u8g2.begin();

  u8g2.clearBuffer();                   // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
  u8g2.drawStr(0, 16, "Hello World!");  // write something to the internal memory
  u8g2.sendBuffer();                    // transfer internal memory to the display

  driveStraight(0.1);
}

void loop() {
   // Move a single white led 
   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::White;

      // Show the leds (only one of which is set to white, from above)
      FastLED.show();

      // Wait a little bit
      delay(100);

      // Turn our current led back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
   }

  motorLeft.run();
  motorRight.run();
}

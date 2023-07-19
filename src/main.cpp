#include <Arduino.h>
#include <ESP_AT_Lib.h>
#include <WS2812B.h>
#include <U8g2lib.h>
#include <AccelStepper.h>

#define EspSerial   Serial

#define SSID        "Obi LAN Kenobi"
#define PASSWORD    "IHaveTheHighGround"

#define HOST_NAME   "10.10.42.193"
#define HOST_PORT   (8000)

#define NUM_LEDS    10

ESP8266 wifi(&EspSerial);

// Your board <-> ESP_AT baud rate:
#define ESP_AT_BAUD       115200

WS2812B strip = WS2812B(NUM_LEDS); // uses SPI1

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

const int stepsPerRevolution = 2048; // maybe x2 when using half-steps
AccelStepper motorLeft(AccelStepper::FULL4WIRE, 0, 1, 2, 3);
AccelStepper motorRight(AccelStepper::FULL4WIRE, 4, 5, 6, 7);

const float WHEEL_RADIUS = 0.0197; // unit: m
const float WHEEL_BASE = 0.0945; // unit: m
float rpmMax = 22;

float distanceToSteps(float distance) {
  float rotations = distance / (WHEEL_RADIUS * 2 * M_PI);
  return stepsPerRevolution * rotations;
}

void driveStraight(float distance) {
  float steps = distanceToSteps(distance);
  motorLeft.setAcceleration(distanceToSteps(0.1)); // does this makes sense?
  motorRight.setAcceleration(distanceToSteps(0.1)); // does this makes sense?
  motorLeft.move(steps);
  motorRight.move(steps);
}

bool isMoving() {
  return motorLeft.isRunning() && motorRight.isRunning();
}

void setup(void)
{
  strip.begin();
  strip.show();

  u8g2.begin();

  motorLeft.setMaxSpeed(rpmMax * stepsPerRevolution);
  motorRight.setMaxSpeed(rpmMax * stepsPerRevolution);

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
}

void loop()
{
  motorLeft.run();
  motorRight.run();
}

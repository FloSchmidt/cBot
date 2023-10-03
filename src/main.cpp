#include <Arduino.h>
#include <ESP_AT_Lib.h>
#include <FastLED.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <AccelStepper.h>
#include <STM32TimerInterrupt.h>

// either "red" or "blu"
#define BOT_TEAM "blu"
#define ACK_MSG "OK"

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

#define BUTTON_UP PC15
#define BUTTON_RIGHT PB3
#define BUTTON_DOWN PB5
#define BUTTON_LEFT PC14

#define MAX_SPEED 200

const int stepsPerRevolution = 2048; // maybe x2 when using half-steps
AccelStepper motorRight(AccelStepper::FULL4WIRE, PA1, PA3, PA0, PA2);
AccelStepper motorLeft(AccelStepper::FULL4WIRE, PA4, PA6, PA5, PA7);

const float WHEEL_RADIUS = 0.0197; // unit: m
const float WHEEL_BASE = 0.0945; // (axle width) unit: m
float rpmMax = 22;

float distanceToSteps(float distance) {
  float rotations = distance / (WHEEL_RADIUS * 2.0 * M_PI);
  return stepsPerRevolution * rotations;
}

void driveStraight(float distance) {
  float steps = distanceToSteps(distance);
  motorLeft.move(steps);
  motorRight.move(steps);
}

void turn(float angle) {
  float arcLenght = (M_PI * WHEEL_BASE) * (angle / 360.0);
  float steps = distanceToSteps(arcLenght);

  motorLeft.move(steps);
  motorRight.move(-steps);
}

bool isMoving() {
  return motorLeft.isRunning() && motorRight.isRunning();
}

// Init STM32 timer TIM1
STM32Timer ITimer1(TIM1);

#define LIGHT_SENSOR_MEAN_SIZE 50
int lightSensorValues[2] = {0, 0};
int lightSensorMeanBuffer[2] = {0, 0};
int lightSensorMeanCounter = 0;
int lightSensorData[2];
int displayCounter = 0;

void MeassureLightHandler() {
  lightSensorValues[0] = analogRead(PB0);
  lightSensorValues[1] = analogRead(PB1);

  // acquire and compute light sensor data
  lightSensorMeanBuffer[0] += lightSensorValues[0];
  lightSensorMeanBuffer[1] += lightSensorValues[1];

  lightSensorMeanCounter++;
  if (lightSensorMeanCounter >= LIGHT_SENSOR_MEAN_SIZE) {
    lightSensorData[0] = (uint16_t)(lightSensorMeanBuffer[0] / LIGHT_SENSOR_MEAN_SIZE);
    lightSensorData[1] = (uint16_t)(lightSensorMeanBuffer[1] / LIGHT_SENSOR_MEAN_SIZE);
    lightSensorMeanBuffer[0] = 0;
    lightSensorMeanBuffer[1] = 0;
    lightSensorMeanCounter = 0;
  }

  // displayCounter++;
  // if (displayCounter >= 1000) {
  //   u8g2.clearBuffer();
  //   u8g2.setFont(u8g2_font_squeezed_r6_tr);
  //   char text[64];
  //   sprintf(text, "left: %d", lightSensorData[0]);
  //   u8g2.drawStr(0, 8, text);
  //   sprintf(text, "right: %d", lightSensorData[1]);
  //   u8g2.drawStr(0, 16, text);
  //   u8g2.sendBuffer();
  //
  //   displayCounter = 0;
  // }
}

static uint8_t mux_id = 0;

void setup() {
  // set the hw i2c pins for u8g2 to PB8 and PB9, because the default is PB6 and PB7
  Wire.setSDA(PB9);
  Wire.setSCL(PB8);
  u8g2.begin();
  u8g2.clearBuffer();

  pinMode(PB0, INPUT);
  pinMode(PB1, INPUT);

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);

  motorLeft.setMaxSpeed(MAX_SPEED);
  motorRight.setMaxSpeed(MAX_SPEED);
  motorLeft.setAcceleration(distanceToSteps(0.1)); // does this makes sense?
  motorRight.setAcceleration(distanceToSteps(0.1)); // does this makes sense?
  // motorLeft.setSpeed(MAX_SPEED);
  // motorRight.setSpeed(MAX_SPEED);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_squeezed_r6_tr);
  u8g2.drawStr(0, 8, "Connecting to WIFI...");
  u8g2.sendBuffer();

  // initialize serial for ESP module
  EspSerial.begin(ESP_AT_BAUD);

  wifi.setOprToStation();
  wifi.joinAP(SSID, PASSWORD);
  wifi.enableMUX();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_squeezed_r6_tr);
  char text[64];
  sprintf(text, "Connecting to %s", HOST_NAME);
  u8g2.drawStr(0, 8, text);
  u8g2.sendBuffer();

  wifi.createTCP(mux_id, HOST_NAME, HOST_PORT);

  char teamMsg[] = BOT_TEAM;
  wifi.send(mux_id, (const uint8_t*)teamMsg, strlen(teamMsg));

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  CRGB teamColor = BOT_TEAM == "red" ? CRGB::Red : CRGB::Blue;
  // light up all LEDs except the last two ones (they are used for the light sensors)
  for(int led = 0; led < NUM_LEDS - 2; led++) {
     leds[led] = teamColor;
  }
  leds[NUM_LEDS-2] = CRGB::White;
  leds[NUM_LEDS-1] = CRGB::White;
  FastLED.show();

  // Interval in microsecs
  ITimer1.attachInterruptInterval(1000, MeassureLightHandler);
}

enum State {
  IDLE,
  FORWARD,
  FORWARD_CROSSING,
  FORWARD_TO_LINE,
  TURN_LEFT,
  TURN_RIGHT,
};

State state = IDLE;
int forwardCount = 0;

void loop() {
  int diff;
  uint8_t command[1] = {0};
  char msg[] = ACK_MSG;

  switch (state) {
    case IDLE:
      motorLeft.setSpeed(0);
      motorRight.setSpeed(0);

      wifi.recv(mux_id, command, sizeof(command), /* timeout */ 100);

      switch (command[0]) {
        case '1':
          forwardCount = 1;
          state = FORWARD;
          break;
        case '2':
          forwardCount = 2;
          state = FORWARD;
          break;
        case '3':
          forwardCount = 3;
          state = FORWARD;
          break;
        case 'r':
          turn(90.0);
          state = TURN_LEFT;
          break;
        case 'l':
          turn(-90.0);
          state = TURN_LEFT;
          break;
        case 's':
        case 'k':
        default:
          break;
      }
      break;
    case FORWARD:
      diff = lightSensorData[0] - lightSensorData[1];

      if (diff < -50) {
        motorLeft.setSpeed(0);
        motorRight.setSpeed(MAX_SPEED);
      } else if (diff > 50) {
        motorLeft.setSpeed(MAX_SPEED);
        motorRight.setSpeed(0);
      } else {
        motorLeft.setSpeed(MAX_SPEED);
        motorRight.setSpeed(MAX_SPEED);
      }

      if ((lightSensorData[0] + lightSensorData[1]) < 180) {
        state = FORWARD_CROSSING;
      }
      break;
    case FORWARD_CROSSING:
      motorLeft.setSpeed(MAX_SPEED);
      motorRight.setSpeed(MAX_SPEED);

      if ((lightSensorData[0] + lightSensorData[1]) > 200) {
        // repeat driving forward and crossing the line until forwardCount is zero
        if (--forwardCount) {
          state = FORWARD;
        } else {
          motorLeft.setSpeed(MAX_SPEED);
          motorRight.setSpeed(MAX_SPEED);
          driveStraight(0.035);
          state = FORWARD_TO_LINE;
        }
      }
      break;
    case FORWARD_TO_LINE:
      // don't use '&&', so that both left and right are evaluated
      if (!(motorLeft.run() & motorRight.run())) {
        wifi.send(mux_id, (const uint8_t*)msg, strlen(msg));
        state = IDLE;
      }
      break;
    case TURN_LEFT:
      // don't use '&&', so that both left and right are evaluated
      if (!(motorLeft.run() & motorRight.run())) {
        wifi.send(mux_id, (const uint8_t*)msg, strlen(msg));
        state = IDLE;
      }
      break;
    case TURN_RIGHT:
      // don't use '&&', so that both left and right are evaluated
      if (!(motorLeft.run() & motorRight.run())) {
        wifi.send(mux_id, (const uint8_t*)msg, strlen(msg));
        state = IDLE;
      }
      break;
  }

  motorLeft.runSpeed();
  motorRight.runSpeed();
}

/*
 * cBotApp.c
 *
 * WallBot
 *
 * Mit diesem Programm kann der cBot einer Wand auf der linken Seite folgen.
 * Der linke Sensor ist dafür im 90° Winkel nach links zu stellen, der rechte ca. 45° nach links.
 *
 * Copyright (C) 2021  Stefan Hoermann (mail@stefan-hoermann.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "cBotApp.h"
#include "cBot.h"
#include "ws2812b.h"
#include "sercom.h"

#include <stdio.h>
#include <math.h>
#include <string.h>


#define WALL_DISTANCE 90
#define OFFSET_SENSOR_RIGHT 30
#define OFFSET_SENSOR_LEFT 45
#define MAX_RANGE 500

extern ws2812b_t *rgbLeds;
extern buttonId buttonRight;
extern sercom_t *serial;

char yPos = 0;

uint32_t blinkTimer = 0;
uint32_t serialTimer = 0;

float remap(float t, float from0, float from1, float to0, float to1)
{
	return (t - from0) / (to0 - from0) * (to1 - from1) + from1;
}

float clamp(float value, float min, float max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

void updateDisplay() {
	// read range values
	int rangeLeft = getRangeMm(SENSOR_LEFT);
	int rangeMiddle = getRangeMm(SENSOR_MIDDLE);
	int rangeRight = getRangeMm(SENSOR_RIGHT);

	// update display
	u8g2_ClearBuffer(display);
	u8g2_SetDrawColor(display, 1);
	u8g2_SetFont(display, u8g2_font_t0_16b_mr);

	// create range text
	char text[64], rangeTextLeft[16], rangeTextMiddle[16], rangeTextRight[16];

	if ( rangeLeft <= MAX_RANGE ) sprintf(rangeTextLeft, "%3d", rangeLeft);
	else sprintf(rangeTextLeft, "---");
	if ( rangeMiddle <= MAX_RANGE ) sprintf(rangeTextMiddle, "%3d", rangeMiddle);
	else sprintf(rangeTextMiddle, "---");
	if ( rangeRight <= MAX_RANGE ) sprintf(rangeTextRight, "%3d", rangeRight);
	else sprintf(rangeTextRight, "---");


	sprintf(text, "ranges");
	u8g2_DrawStr(display, 0, 16, text);

	sprintf(text, "%s|%s|%s", rangeTextLeft, rangeTextMiddle, rangeTextRight);
	u8g2_DrawStr(display, (128 - u8g2_GetStrWidth(display, text))/2, 32, text);

	sprintf(text, "line sensors");
	u8g2_DrawStr(display, 0, 48, text);

	sprintf(text, "%4d|%4d", getLightValue(SENSOR_LEFT), getLightValue(SENSOR_RIGHT));
	u8g2_DrawStr(display, (128 - u8g2_GetStrWidth(display, text))/2, 64, text);

	u8g2_SendBuffer(display);
}

void setLightSensorColor(uint32_t color)
{
	ws2812b_setColor(rgbLeds, 8, color);
	ws2812b_setColor(rgbLeds, 9, color);
	ws2812b_update(rgbLeds);
}

void cycleLightSensorColor()
{
	static int currentColorIndex = 0;
	static const uint32_t colors[] = {
			CONSTEXPR_COLOR(255, 255, 255),
			CONSTEXPR_COLOR(0, 0, 0),
			CONSTEXPR_COLOR(0, 0, 255),
			CONSTEXPR_COLOR(0, 255, 0),
			CONSTEXPR_COLOR(255, 0, 0),
	};
	static const int colorsCount = 5;
	setLightSensorColor(colors[currentColorIndex]);
	//updateLeds();
	currentColorIndex = (currentColorIndex + 1) % colorsCount;
}

void clearInitDisplay()
{
	yPos = 0;
	u8g2_ClearBuffer(display);
	u8g2_SetDrawColor(display, 1);
	u8g2_SetFont(display, u8g2_font_5x7_tf);
}

void writeSerialToDisplay(char *text) {
	u8g2_DrawStr(display, 0, yPos += 6, text);
	u8g2_SendBuffer(display);
}

void init() {
	// initialize your cBot here
//	sercom_transmitStr(serial, "AT\r\n");
//	while(serial->isTxActive);
//	HAL_Delay(100);

//	while(serial->isTxActive);
//	HAL_Delay(500);
//	char c[64] = {0};
//	sercom_readLine(serial, c, 64);
//
// 	sercom_transmitStr(serial, "AT+CWMODE=2\r\n");
// 	while(serial->isTxActive);
// 	HAL_Delay(100);
// 	while(serial->isTxActive);

// 	while(sercom_bytesAvailable(serial) < 2);
//
//	sercom_readLine(serial, c, 64);

	char temp[100];
	HAL_Delay(1000);
	sercom_transmitStr(serial, "AT+RST\r\n");
	HAL_Delay(1000);
	clearInitDisplay();
	while (sercom_linesAvailable(serial))
	{
		memset(temp,'\0',100);
		sercom_readLine(serial, temp, sizeof(temp)-1);
		writeSerialToDisplay(temp);
		if (temp[0]=='r' && temp[1]=='e' && temp[2]=='a' && temp[3]=='d' && temp[4]=='y')
		{
			break;
		}
		else
		{
			HAL_Delay(1000);
		}
	}

	sercom_transmitStr(serial, "AT+CWMODE=3\r\n");
	HAL_Delay(1000);
	while (sercom_linesAvailable(serial))
	{
		memset(temp,'\0',100);
		sercom_readLine(serial, temp, sizeof(temp)-1);
		writeSerialToDisplay(temp);
		if (temp[0]=='O' && temp[1]=='K')
		{
			break;
		}
		else
		{
			HAL_Delay(1000);
		}
	}

	clearInitDisplay();
	sercom_transmitStr(serial, "AT+CWJAP=\"NewTonWars\",\"AchPatrickAch\"\r\n");
	HAL_Delay(1000);
	while (sercom_linesAvailable(serial))
	{
		memset(temp,'\0',100);
		sercom_readLine(serial, temp, sizeof(temp)-1);
		writeSerialToDisplay(temp);
		if (temp[0]=='O' && temp[1]=='K')
		{
			break;
		}
		else
		{
			HAL_Delay(1000);
		}
	}
	sercom_transmitStr(serial, "AT+CIPSTART=\"TCP\",\"192.168.2.102\",4321\r\n");
	HAL_Delay(1000);
	while (sercom_linesAvailable(serial))
	{
		memset(temp,'\0',100);
		sercom_readLine(serial, temp, sizeof(temp)-1);
		writeSerialToDisplay(temp);
		if (temp[0]=='O' && temp[1]=='K')
		{
			break;
		}
		else
		{
			HAL_Delay(1000);
		}
	}

	clearInitDisplay();
	sercom_transmitStr(serial, "AT+CIPSEND=14\r\n");
	HAL_Delay(1000);
	while (sercom_linesAvailable(serial))
	{
		memset(temp,'\0',100);
		sercom_readLine(serial, temp, sizeof(temp)-1);
		writeSerialToDisplay(temp);
		if (temp[0]=='O' && temp[1]=='K')
		{
			break;
		}
		else
		{
			HAL_Delay(1000);
		}
	}

	char payload[] = "Hello World!\r\n";
	sercom_transmitStr(serial, "Hello World!\r\n");
	HAL_Delay(1000);
	while (sercom_linesAvailable(serial))
	{
		memset(temp,'\0',100);
		sercom_readLine(serial, temp, sizeof(temp)-1);
		writeSerialToDisplay(temp);
		if (temp[0]=='S' && temp[1]=='E' && temp[2]=='N' && temp[3]=='D' && temp[4]==' ' && temp[5]=='O' && temp[6]=='K')
		{
			break;
		}
		else
		{
			HAL_Delay(1000);
		}
	}

	cycleLightSensorColor();
	robotMachine_Init();
}


#define ERROR_HISTORY_SIZE 100

int running = 0;
int lastTurnLeft = 0;

int errorHistory[ERROR_HISTORY_SIZE], historyId;

void resetErrorHistory() {
	for ( int i = ERROR_HISTORY_SIZE - 1; i >= 0; i-- ) errorHistory[i] = 0;
	historyId = 0;
}

void loop() {
// read range values
//	int rangeLeft = getRangeMm(SENSOR_LEFT);
//	int rangeMiddle = getRangeMm(SENSOR_MIDDLE);
//	int rangeRight = getRangeMm(SENSOR_RIGHT);
	const int intensityL = getLightValue(SENSOR_LEFT);
	const int intensityR = getLightValue(SENSOR_RIGHT);

	static int ledstate = 0;

	if(millistimer_expired(&blinkTimer, 500))
	{
		if(ledstate)
		{
			ledstate = 0;
			setLed(1, getColorRGB(255,0, 0));
		}
		else
		{
			ledstate = 1;
			setLed(1, getColorRGB(0, 255, 0));
		}

		updateLeds();
	}

	if(sercom_linesAvailable(serial))
	{
		char data[100];
		sercom_readLine(serial, data, 99);

		char expected[] = "+IPD,2:";
		if (memcmp(data, expected, sizeof(expected)));
		{
			if (data[7] == 'l' || data[7] == 'L')
			{
				robotMachine_EVENT_Left();
			}

			if (data[7] == 'r' || data[7] == 'R')
			{
				robotMachine_EVENT_Right();
			}

			if (data[7] == '1')
			{
				robotMachine_EVENT_Forward(1);
			}

			if (data[7] == '2')
			{
				robotMachine_EVENT_Forward(2);
			}

			if (data[7] == '3')
			{
				robotMachine_EVENT_Forward(3);
			}
			sercom_transmitStr(serial, data);
		}
	}

	if ( isPressed(BUTTON_LEFT) ) {
		cycleLightSensorColor();
		while ( isPressed(BUTTON_LEFT) );	// wait until key is released
	}

	if ( isPressed(BUTTON_RIGHT) ) {
		running = !running;
		if ( !running ) {
			stopMotor();
		}
		while ( isPressed(BUTTON_RIGHT) );	// wait until key is released
	}

	static int servoPos = 100;
	if ( isPressed(BUTTON_UP) ) {
		if (servoPos == 100)
		{
			servoPos = 900;
		}
		else
		{
			servoPos = 100;
		}

		setServo(1, servoPos);
		while ( isPressed(BUTTON_UP) );	// wait until key is released
	}


//	if (running)
//	{
//	    int diff = intensityL - intensityR;
//
//	    float speedRight = clamp(remap(diff, 100, 0, 16, 0), 0, 16);
//	    float speedLeft = clamp(remap(diff, -100, 0, 0, 16), 0, 16);
//
//	    setMotorRpm(speedLeft, speedRight);
//	}




	robotMachine_Task();


//	if (running && !isMoving())
//	{
//		ws2812b_setColor(rgbLeds, 0, ws2812b_colorRGB(255, 0, 0));
//		if (lastTurnLeft)
//		{
//			turn(30);
//			lastTurnLeft = 0;
//		}
//		else
//		{
//			turn(-30);
//			lastTurnLeft = 1;
//		}
//	}

	//ws2812b_update(rgbLeds);

//	if ( !isWaiting ) {
//		const int diff = intensityL - intensityR;
//		const float base = 20.;
//
//		const float rpmLeft = base * (diff / 600);
//		const float rpmRight = base * (- diff / 600);

//		motorUpdate();
//
//		getRpmFromVelocity(&rpmLeft, &rpmRight, 0.02, -angularSpeed); // linear velocity in m/s; angular rate in radians/s
//		setMotorRpm(rpmLeft, rpmRight);

//
//		if ( (rangeLeft > MAX_RANGE) && (rangeMiddle > MAX_RANGE) && (rangeRight > MAX_RANGE) ) {
//			driveArc((WALL_DISTANCE + OFFSET_SENSOR_LEFT) / 1000.0, 30); // radius in m, angle in degree
//			while ( isMoving() );
//			resetErrorHistory();
//		}
//		else
//			if ( (rangeMiddle <= WALL_DISTANCE) || (rangeRight - OFFSET_SENSOR_RIGHT <= WALL_DISTANCE) ) {
//			turn(-15);
//			while ( isMoving() );
//			resetErrorHistory();
//		}
////		else if (rangeLeft <= MAX_RANGE) {
//
//			// determine error
//			int error = WALL_DISTANCE - rangeLeft;
//			if ( error < -WALL_DISTANCE ) error = -WALL_DISTANCE;
//
//			// get old error and store current error
//			int errorOld = errorHistory[historyId];
//			errorHistory[historyId] = error;
//			historyId = (historyId + 1) % ERROR_HISTORY_SIZE;
//			if ( historyId == 0 ) HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//
//			float angularSpeed = 0.002 * error + 0.002 * (error - errorOld);
//
//			float rpmLeft, rpmRight;
//			getRpmFromVelocity(&rpmLeft, &rpmRight, 0.02, -angularSpeed); // linear velocity in m/s; angular rate in radians/s
//			setMotorRpm(rpmLeft, rpmRight);
//		}
//	}
//	else {
//		setMotorRpm(0, 0);
//	}
//
//	// start or stop robot
//	if ( isPressed(BUTTON_RIGHT) ) {
//		isWaiting = 1 - isWaiting;
//		if ( !isWaiting ) resetErrorHistory();
//		while ( isPressed(BUTTON_RIGHT) );	// wait until key is released
//	}
//
	updateDisplay();

}



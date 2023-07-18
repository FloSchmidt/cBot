#include "communication_machine.h"
#include "sercom.h"
#include <string.h>
#include "arrayhelper.h"
#include "u8g2.h"
#include "cbot.h"
#include <stdio.h>

uint32_t resetWait;


const char ssid[] = "NewTonWars";
const char pwd[] = "AchPatrickAch";
const char protocol[] = "TCP";
const char server[] = "192.168.2.102";
const char port[] = "4321";
const char hello[] = "Hello World!";
int networkConnected = 0;
char lastLine[100];
int yPos = 0;
extern u8g2_t *display;
extern sercom_t *serial;

void clearInitDisplay()
{
	yPos = 0;
	u8g2_ClearBuffer(display);
	u8g2_SetDrawColor(display, 1);
	u8g2_SetFont(display, u8g2_font_5x7_tf);
}

void writeLineDisplay(char *text) {
	u8g2_DrawStr(display, 0, yPos += 6, text);
	u8g2_SendBuffer(display);
}

void clearInputBuffer()
{
	while(sercom_bytesAvailable(serial))
	{
		sercom_readByte(serial);
	}
	lastLine[0] = '\0';
}

void communicationMachine_InitState(SM_StateMachine *self);

void communicationMachine_ResetEsp(SM_StateMachine *self);
void communicationMachine_StateEnter_ResetEsp(SM_StateMachine *self);

void communicationMachine_WifiMode(SM_StateMachine *self);
void communicationMachine_StateEnter_WifiMode(SM_StateMachine *self);

void communicationMachine_ConnectWifi(SM_StateMachine *self);
void communicationMachine_StateEnter_ConnectWifi(SM_StateMachine *self);

void communicationMachine_ConnectSocket(SM_StateMachine *self);
void communicationMachine_StateEnter_ConnectSocket(SM_StateMachine *self);

void communicationMachine_Idle(SM_StateMachine *self);
void communicationMachine_StateEnter_Idle(SM_StateMachine *self);

void communicationMachine_PrepareSend(SM_StateMachine *self);
void communicationMachine_StateEnter_PrepareSend(SM_StateMachine *self);

void communicationMachine_SendData(SM_StateMachine *self);
void communicationMachine_StateEnter_SendData(SM_StateMachine *self);

int lastReceivedLineOK() {
	return strcmp(lastLine,"OK\r") == 0;
}

int lastReceivedLineSendOK() {
	return strcmp(lastLine,"SEND OK\r") == 0;
}

int lastReceivedLineReady() {
	return strcmp(lastLine,"ready\r") == 0;
}

SM_StateFktEntry communicationMachine_stateFunctions[] = {
	{communicationMachine_InitState, NULL, NULL },
	{communicationMachine_ResetEsp, communicationMachine_StateEnter_ResetEsp, NULL },
	{communicationMachine_WifiMode, communicationMachine_StateEnter_WifiMode, NULL },
	{communicationMachine_ConnectWifi, communicationMachine_StateEnter_ConnectWifi, NULL },
	{communicationMachine_ConnectSocket, communicationMachine_StateEnter_ConnectSocket, NULL},
	{communicationMachine_Idle, communicationMachine_StateEnter_Idle, NULL},
	{communicationMachine_PrepareSend, communicationMachine_StateEnter_PrepareSend, NULL},
	{communicationMachine_SendData, communicationMachine_StateEnter_SendData, NULL},
};

enum State {
	INIT_STATE,
	RESET_ESP,
	WIFI_MODE,
	CONNECT_WIFI,
	CONNECT_SOCKET,
	IDLE,
	PREPARE_SEND,
	SEND_DATA,
};

const SM_StateMachineConst communicationMachineConst = {
	COUNT_ELEMENTS(communicationMachine_stateFunctions),
	communicationMachine_stateFunctions,
	"communicationSM"
};

SM_StateMachine communicationMachine = {
	INIT_STATE,
	&communicationMachineConst
};

void communicationMachine_Init(void)
{
	clearInitDisplay();
	stateMachine_Init(&communicationMachine);
}

void communicationMachine_Task(void)
{
	if (sercom_linesAvailable(serial))
	{
		memset(lastLine, 0, sizeof(lastLine));
		sercom_readLine(serial, lastLine, sizeof(lastLine)-1);
		printf("ESP>>: %s", lastLine);
	}

	stateMachine_Task(&communicationMachine);
}

void communicationMachine_InitState(SM_StateMachine *self)
{
	stateMachine_transissionTo(self, RESET_ESP);
}

void communicationMachine_ResetEsp(SM_StateMachine *self)
{
	if(millistimer_expired(&resetWait, 10000))
		stateMachine_transissionTo(self, WIFI_MODE);
}

void communicationMachine_StateEnter_ResetEsp(SM_StateMachine *self)
{
	writeLineDisplay("resetEsp");
	sercom_transmitStr(serial, "AT+RST\r\n");
	printf("ESP<<: %s", "AT+RST\r\n");
	millistimer_start(&resetWait);
}

void communicationMachine_WifiMode(SM_StateMachine *self)
{
	if(lastReceivedLineOK())
		stateMachine_transissionTo(self, CONNECT_WIFI);
}

void communicationMachine_StateEnter_WifiMode(SM_StateMachine *self)
{
	writeLineDisplay("wifimode to 3");
	clearInputBuffer();
	sercom_transmitStr(serial, "AT+CWMODE=3\r\n");
	printf("ESP<<: %s", "AT+CWMODE=3\r\n");
	millistimer_start(&resetWait);
}

void communicationMachine_ConnectWifi(SM_StateMachine *self)
{
	if(lastReceivedLineOK())
		stateMachine_transissionTo(self, CONNECT_SOCKET);
}
void communicationMachine_StateEnter_ConnectWifi(SM_StateMachine *self)
{
	char commandPattern[] = "AT+CWJAP=\"%s\",\"%s\"\r\n";
	char buffer[50];
	clearInputBuffer();

	snprintf(buffer, sizeof(buffer), commandPattern, ssid, pwd);
	writeLineDisplay(buffer);
	printf(buffer);

	sercom_transmitStr(serial, buffer);
}

void communicationMachine_ConnectSocket(SM_StateMachine *self)
{
	if(lastReceivedLineOK())
		stateMachine_transissionTo(self, IDLE);
}

void communicationMachine_StateEnter_ConnectSocket(SM_StateMachine *self)
{
	char commandPattern[] = "AT+CIPSTART=\"%s\",\"%s\",%s\r\n";
	char buffer[50];
	clearInputBuffer();

	snprintf(buffer, sizeof(buffer), commandPattern, protocol, server, port);
	writeLineDisplay(buffer);
	printf(buffer);

	sercom_transmitStr(serial, buffer);
}

void communicationMachine_Idle(SM_StateMachine *self)
{
	if(millistimer_expired(&resetWait, 1000))
		stateMachine_transissionTo(self, PREPARE_SEND);
}

void communicationMachine_StateEnter_Idle(SM_StateMachine *self)
{
	clearInputBuffer();
	millistimer_start(&resetWait);
}

void communicationMachine_PrepareSend(SM_StateMachine *self)
{
	if(lastReceivedLineOK())
		stateMachine_transissionTo(self, SEND_DATA);
}

void communicationMachine_StateEnter_PrepareSend(SM_StateMachine *self)
{
	clearInputBuffer();
	char commandPattern[] = "AT+CIPSEND=%u\r\n";
	char buffer[50];

	snprintf(buffer, sizeof(buffer), commandPattern, strlen(hello));
	writeLineDisplay(buffer);
	printf(buffer);

	sercom_transmitStr(serial, buffer);
}

void communicationMachine_SendData(SM_StateMachine *self)
{
	if(lastReceivedLineSendOK())
		stateMachine_transissionTo(self, IDLE);
}

void communicationMachine_StateEnter_SendData(SM_StateMachine *self)
{
	clearInputBuffer();
	sercom_transmitStr(serial, hello);
}



//clearInitDisplay();
//	sercom_transmitStr(serial, "AT+CIPSEND=14\r\n");
//	HAL_Delay(1000);
//	while (sercom_linesAvailable(serial))
//	{
//		memset(temp,'\0',100);
//		sercom_readLine(serial, temp, sizeof(temp)-1);
//		writeSerialToDisplay(temp);
//		if (temp[0]=='O' && temp[1]=='K')
//		{
//			break;
//		}
//		else
//		{
//			HAL_Delay(1000);
//		}
//	}
//
//	char payload[] = "Hello World!\r\n";
//	sercom_transmitStr(serial, "Hello World!\r\n");
//	HAL_Delay(1000);
//	while (sercom_linesAvailable(serial))
//	{
//		memset(temp,'\0',100);
//		sercom_readLine(serial, temp, sizeof(temp)-1);
//		writeSerialToDisplay(temp);
//		if (temp[0]=='S' && temp[1]=='E' && temp[2]=='N' && temp[3]=='D' && temp[4]==' ' && temp[5]=='O' && temp[6]=='K')
//		{
//			break;
//		}
//		else
//		{
//			HAL_Delay(1000);
//		}
//	}


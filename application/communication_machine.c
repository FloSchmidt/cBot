#include "communication_machine.h"
#include "sercom.h"
#include <string.h>
#include "arrayhelper.h"
#include "u8g2.h"
#include "cbot.h"

uint32_t resetWait;

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

void communicationMachine_PrepareSend(SM_StateMachine *self);
void communicationMachine_StateEnter_PrepareSend(SM_StateMachine *self);

void communicationMachine_SendData(SM_StateMachine *self);
void communicationMachine_StateEnter_SendData(SM_StateMachine *self);

int lastReceivedLineOK() {
	return strcmp(lastLine,"OK\r\n") == 0;
}

int lastReceivedLineSendOK() {
	return strcmp(lastLine,"SEND OK\r\n") == 0;
}

int lastReceivedLineReady() {
	return strcmp(lastLine,"ready\r\n") == 0;
}

SM_StateFktEntry communicationMachine_stateFunctions[] = {
	{communicationMachine_InitState, NULL, NULL },
	{communicationMachine_ResetEsp, communicationMachine_StateEnter_ResetEsp, NULL },
	{communicationMachine_WifiMode, communicationMachine_StateEnter_WifiMode, NULL },
	{communicationMachine_ConnectWifi, communicationMachine_StateEnter_ConnectWifi, NULL },
	{communicationMachine_ConnectSocket, communicationMachine_StateEnter_ConnectSocket, NULL},
	{communicationMachine_Idle, NULL, NULL},
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
	}

	stateMachine_Task(&communicationMachine);
}

void communicationMachine_InitState(SM_StateMachine *self)
{
	stateMachine_transissionTo(self, RESET_ESP);
}

void communicationMachine_ResetEsp(SM_StateMachine *self)
{
	if(millistimer_expired(&resetWait, 5000))
		stateMachine_transissionTo(self, WIFI_MODE);
}

void communicationMachine_StateEnter_ResetEsp(SM_StateMachine *self)
{
	writeLineDisplay("resetEsp");
	sercom_transmitStr(serial, "AT+RST\r\n");
	millistimer_start(&resetWait);
}

void communicationMachine_WifiMode(SM_StateMachine *self)
{
	if(lastReceivedLineOK() || millistimer_expired(&resetWait, 2000))
		stateMachine_transissionTo(self, CONNECT_WIFI);
}

void communicationMachine_StateEnter_WifiMode(SM_StateMachine *self)
{
	writeLineDisplay("wifimode to 3");
	sercom_transmitStr(serial, "AT+CWMODE=3\r\n");
	millistimer_start(&resetWait);
}

void communicationMachine_ConnectWifi(SM_StateMachine *self)
{
	if(lastReceivedLineOK())
		stateMachine_transissionTo(self, CONNECT_SOCKET);
}
void communicationMachine_StateEnter_ConnectWifi(SM_StateMachine *self)
{
	writeLineDisplay("connect to wifi");
	sercom_transmitStr(serial, "AT+CWJAP=\"NewTonWars\",\"AchPatrickAch\"\r\n");
}

void communicationMachine_ConnectSocket(SM_StateMachine *self)
{
	if(lastReceivedLineOK())
		stateMachine_transissionTo(self, IDLE);
}

void communicationMachine_StateEnter_ConnectSocket(SM_StateMachine *self)
{
	writeLineDisplay("connect to server");
	sercom_transmitStr(serial, "AT+CIPSTART=\"TCP\",\"192.168.2.102\",4321\r\n");
}

void communicationMachine_Idle(SM_StateMachine *self)
{

}

void communicationMachine_PrepareSend(SM_StateMachine *self)
{

}

void communicationMachine_StateEnter_PrepareSend(SM_StateMachine *self)
{

}

void communicationMachine_SendData(SM_StateMachine *self)
{

}

void communicationMachine_StateEnter_SendData(SM_StateMachine *self)
{

}



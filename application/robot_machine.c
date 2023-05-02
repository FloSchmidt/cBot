#include "robot_machine.h"
#include "arrayhelper.h"
#include "button.h"

#include <stdio.h>
#include <stddef.h>
#include <cBot.h>

extern button_t buttonDown;

void robotMachine_StateLoop_Idle(SM_StateMachine *self);
void robotMachine_StateLoop_IdlePressed(SM_StateMachine *self);
void robotMachine_StateLoop_Forward(SM_StateMachine *self);
void robotMachine_StateLoop_ForwardCrossing(SM_StateMachine *self);
void robotMachine_StateLoop_ForwardToLine(SM_StateMachine *self);
void robotMachine_StateEnter_ForwardToLine(SM_StateMachine *self);
void robotMachine_StateLoop_TurnLeft(SM_StateMachine *self);
void robotMachine_StateEnter_TurnLeft(SM_StateMachine *self);

SM_StateFktEntry stateFunctions[] = {
	{robotMachine_StateLoop_Idle, NULL, NULL },
	{robotMachine_StateLoop_IdlePressed, NULL, NULL },
	{robotMachine_StateLoop_Forward, NULL, NULL},
	{robotMachine_StateLoop_ForwardCrossing, NULL, NULL},
	{robotMachine_StateLoop_ForwardToLine, robotMachine_StateEnter_ForwardToLine, NULL},
	{robotMachine_StateLoop_TurnLeft, robotMachine_StateEnter_TurnLeft, NULL},
};

enum State {
	IDLE,
	IDLE_PRESSED,
	FORWARD,
	FORWARD_CROSSING,
	FORWARD_TO_LINE,
	TURN_LEFT,
};

const SM_StateMachineConst robotMachineConst = {
	COUNT_ELEMENTS(stateFunctions),
	stateFunctions,
	"robotSM"
};

SM_StateMachine robotMachine = {
	IDLE,
	&robotMachineConst
};

void robotMachine_Init(void)
{
	stateMachine_Init(&robotMachine);
}

void robotMachine_Task(void)
{
	stateMachine_Task(&robotMachine);
}

void robotMachine_StateLoop_Idle(SM_StateMachine *self)
{
	setMotorRpm(0,0);
	if (isPressed(BUTTON_DOWN))
		stateMachine_transissionTo(self, IDLE_PRESSED);
}

void robotMachine_StateLoop_IdlePressed(SM_StateMachine *self)
{
	if(!isPressed(BUTTON_DOWN))
		stateMachine_transissionTo(self, FORWARD);
}

void robotMachine_StateLoop_Forward(SM_StateMachine *self)
{
	const int intensityL = getLightValue(SENSOR_LEFT);
	const int intensityR = getLightValue(SENSOR_RIGHT);

	int diff = intensityL - intensityR;

	if (diff < -100)
		setMotorRpm(0,8);
	else if (diff > 100)
		setMotorRpm(8,0);
	else
		setMotorRpm(8, 8);

	if ((intensityL + intensityR) < 1800)
	{
		stateMachine_transissionTo(self, FORWARD_CROSSING);
	}
}

void robotMachine_StateLoop_ForwardCrossing(SM_StateMachine *self)
{
	const int intensityL = getLightValue(SENSOR_LEFT);
	const int intensityR = getLightValue(SENSOR_RIGHT);
	setMotorRpm(8, 8);
	if ((intensityL + intensityR) > 2000)
	{
		stateMachine_transissionTo(self, FORWARD_TO_LINE);
	}
}

void robotMachine_StateEnter_ForwardToLine(SM_StateMachine *self)
{
	driveStrait(0.04);
}

void robotMachine_StateLoop_ForwardToLine(SM_StateMachine *self)
{
	if(!isMoving())
		stateMachine_transissionTo(self, TURN_LEFT);
}
//
//void robotMachine_StateEnter_Left(SM_StateMachine *self)
//{
//
//}
//

void robotMachine_StateEnter_TurnLeft(SM_StateMachine *self)
{
	turn(90);
}

void robotMachine_StateLoop_TurnLeft(SM_StateMachine *self)
{
	if(!isMoving())
		stateMachine_transissionTo(self, IDLE);
}
//


int robotMachine_EVENT_Forward()
{
	if (robotMachine.currentState != IDLE)
		return 1;

	stateMachine_transissionTo(&robotMachine, FORWARD);
	return 0;
}


//
//void dispenserMachine_StateEnter_Locked(SM_StateMachine *self)
//{
//	dispenserMachine_sendState(0);
//	//tower_lock();
//	doors_lockAll();
//
//	ledStrip_compartmentSwitchOff();
//}
//
//void dispenserMachine_StateLoop_Locked(SM_StateMachine *self)
//{
//	if (!doors_allDoorsAreClosed())
//	{
//		stateMachine_transissionTo(self, ERROR_WRONG_FLAP);
//		return;
//	}
//
//	if (towerUnlocked)
//	{
//		tower_unlock();
//
//		ledStrip_compartmentSwitchOn();
//		if(millistimer_expired(&towerPositionReportTimer, TOWER_POSITION_REPORT_PERIODE))
//		{
//			const uint16_t pos = tower_position();
//			dispenserMachine_sendTowerPos(pos, 0, pos);
//		}
//	}
//	else
//	{
//		tower_lock();
//		ledStrip_compartmentSwitchOff();
//	}
//
//	if(key_unlocked())
//	{
//		stateMachine_transissionTo(self, OPENED_WITH_KEY);
//		return;
//	}
//}
//
//void dispenserMachine_StateEnter_RotateToTarget(SM_StateMachine *self)
//{
//	dispenserMachine_sendState(1);
//}
//
//void dispenserMachine_StateLoop_RotateToTarget(SM_StateMachine *self)
//{
//	const int16_t distance = tower_distance(targetAngle);
//	const uint8_t inLockingRange = tower_distanceInLockingRange(distance);
//
//	if(millistimer_expired(&towerPositionReportTimer, TOWER_POSITION_REPORT_PERIODE))
//	{
//		dispenserMachine_sendTowerPos(tower_position(), distance, targetAngle);
//	}
//
//	inLockingRange ? tower_lock() : tower_unlock();
//
//	if (tower_isLocked() && inLockingRange)
//	{
//		dispenserMachine_sendTowerPos(tower_position(), 0, targetAngle);
//		stateMachine_transissionTo(self, WAIT_FOR_OPEN);
//		return;
//	}
//}
//
//void dispenserMachine_StateEnter_WaitForOpen(SM_StateMachine *self)
//{
//	dispenserMachine_sendState(2);
//	dispenserMachine_sendLayer(targetLayer);
//
//	doors_unlock(targetLayer);
//	ledStrip_compartmentHighlightLayer(targetLayer, doors_doorHeight(targetLayer));
//}
//
//void dispenserMachine_StateLoop_WaitForOpen(SM_StateMachine *self)
//{
//	if(doors_doorIsOpened(targetLayer))
//	{
//		stateMachine_transissionTo(self, WAIT_FOR_CLOSE);
//		return;
//	}
//}
//
//void dispenserMachine_StateEnter_WaitForClose(SM_StateMachine *self)
//{
//	dispenserMachine_sendState(3);
//	dispenserMachine_sendLayer(targetLayer);
//	doors_unlock(targetLayer);
//	//ledStrip_hightHighlightLayer(targetLayer);
//	ledStrip_compartmentHighlightLayer(targetLayer, doors_doorHeight(targetLayer));
//}
//
//void dispenserMachine_StateLoop_WaitForClose(SM_StateMachine *self)
//{
//	if (doors_doorIsClosed(targetLayer))
//	{
//		stateMachine_transissionTo(self, WAIT_FOR_CLOSE_DELAY);
//		return;
//	}
//}
//
//void dispenserMachine_StateEnter_WaitForCloseDelay(SM_StateMachine *self)
//{
//	dispenserMachine_sendState(4);
//	dispenserMachine_sendLayer(targetLayer);
//	doors_unlock(targetLayer);
//	//ledStrip_hightHighlightLayer(targetLayer);
//	ledStrip_compartmentHighlightLayer(targetLayer, doors_doorHeight(targetLayer));
//	millistimer_delay(&doorsAreClosedDelay, DOOOR_CLOSED_DELAY);
//}
//
//void dispenserMachine_StateLoop_WaitForCloseDelay(SM_StateMachine *self)
//{
//	if (!doors_allDoorsAreClosed())
//	{
//		stateMachine_transissionTo(self, WAIT_FOR_CLOSE);
//		return;
//	}
//	if (millistimer_expired(&doorsAreClosedDelay, DOOOR_CLOSED_DELAY))
//	{
//		stateMachine_transissionTo(self, CLOSING);
//		return;
//	}
//}
//
//void dispenserMachine_StateEnter_ErrorWrongFlap(SM_StateMachine *self)
//{
//	dispenserMachine_sendState(5);
//	tower_lock();
//
//	currentWrongDoor = doors_upmostOpenedDoor();
//	doors_unlock(currentWrongDoor);
//
//	ledStrip_compartmentErrorLayer(currentWrongDoor, doors_doorHeight(currentWrongDoor));
//}
//
//void dispenserMachine_StateLoop_ErrorWrongFlap(SM_StateMachine *self)
//{
//	if(doors_doorIsClosed(currentWrongDoor))
//	{
//		stateMachine_transissionTo(self, ERROR_WRONG_FLAP_DELAY);
//		return;
//	}
//}
//
//void dispenserMachine_StateEnter_ErrorWrongFlapDelay(SM_StateMachine *self)
//{
//	dispenserMachine_sendState(6);
//	dispenserMachine_sendLayer(targetLayer);
//
//	millistimer_delay(&doorsAreClosedDelay, DOOOR_CLOSED_DELAY);
//}
//
//void dispenserMachine_StateLoop_ErrorWrongFlapDelay(SM_StateMachine *self)
//{
//	if (doors_doorIsOpened(currentWrongDoor))
//	{
//		stateMachine_transissionTo(self, ERROR_WRONG_FLAP);
//		return;
//	}
//
//	if (millistimer_expired(&doorsAreClosedDelay, DOOOR_CLOSED_DELAY))
//	{
//		stateMachine_transissionTo(self, CLOSING);
//		return;
//	}
//}
//
//void dispenserMachine_StateEnter_OpenedWithKey(SM_StateMachine *self)
//{
//	dispenserMachine_sendState(7);
//	millistimer_delay(&keyOpenedDelay, KEY_OPENED_DELAY);
//
//	doors_unlockAll();
//
//
//	//ledStrip_heightGreen();
//	ledStrip_compartmentSwitchOn();
//}
//
//void dispenserMachine_StateLoop_OpenedWithKey(SM_StateMachine *self)
//{
//	if(millistimer_expired(&towerPositionReportTimer, TOWER_POSITION_REPORT_PERIODE))
//	{
//		dispenserMachine_sendTowerPos(tower_position(), 0, targetAngle);
//	}
//
//	if(millistimer_expired(&keyOpenedDelay,0))
//	{
//		tower_lock();
//	}
//	else
//	{
//		tower_unlock();
//	}
//
//	if(!key_unlocked())
//	{
//		stateMachine_transissionTo(self, CLOSING);
//		return;
//	}
//}
//
//void dispenserMachine_StateEnter_Closing(SM_StateMachine *self)
//{
//	doors_lockAll();
//	ledStrip_compartmentSwitchOff();
//	millistimer_delay(&closingDelay, CLOSING_DELAY);
//}
//
//void dispenserMachine_StateLoop_Closing(SM_StateMachine *self)
//{
//	if (millistimer_expired(&closingDelay, CLOSING_DELAY))
//	{
//		stateMachine_transissionTo(self, LOCKED);
//	}
//}
//
//void dispenserMachine_EVENTcancel(void)
//{
//	if (dispenserMachine.currentState == LOCKED)
//	{
//		return;
//	}
//	if(doors_allDoorsAreClosed())
//	{
//		stateMachine_transissionTo(&dispenserMachine, CLOSING);
//		return;
//	}
//	stateMachine_transissionTo(&dispenserMachine, WAIT_FOR_CLOSE);
//	return;
//}
//
//uint8_t dispenserMachine_EVENTopenCompartment(uint8_t column, uint8_t layer)
//{
//	COMMANDPARSER_SENDDEBUG("EVENT OpenCompartment c:%d l:%d", 50, column, layer);
//	if (dispenserMachine.currentState == WAIT_FOR_CLOSE || dispenserMachine.currentState == WAIT_FOR_CLOSE_DELAY)
//	{
//		//ignore event
//		return 0;
//	}
//	targetAngle = tower_positionForIndex(column);
//	targetLayer = layer;
//	stateMachine_transissionTo(&dispenserMachine, ROTATE_TO_TARGET);
//	return 1;
//}
//
//uint8_t dispenserMachine_EVENTenableFreeRotation(void)
//{
//	COMMANDPARSER_SENDDEBUG("EVENT enableFreeRotation", 30);
//	towerUnlocked = 1;
//	return 1;
//}
//
//uint8_t dispenserMachine_EVENTdisableFreeRotation(void)
//{
//	COMMANDPARSER_SENDDEBUG("EVENT disableFreeRotation", 30);
//	towerUnlocked = 0;
//	return 1;
//}
//
//void dispenserMachine_sendState(uint8_t state)
//{
//	uint8_t buffer[20];
//	const uint8_t count = snprintf(buffer, 20, "S%02X", state);
//	commandParser_sendReply(DC1, buffer, count);
//}
//
//void dispenserMachine_sendTowerPos(uint16_t position, int16_t distance, uint16_t target)
//{
//	uint8_t buffer[30];
//	const uint8_t currentIndex = tower_indexForPosition(position);
//	const uint8_t targetIndex = tower_indexForPosition(target);
//	const uint8_t count = snprintf(buffer, 30, "T%04X,%04X,%04X,%02X,%02X", position, distance, target, currentIndex, targetIndex);
//	commandParser_sendReply(DC1, buffer, count);
//}
//
//void dispenserMachine_sendLayer(uint8_t layer)
//{
//	uint8_t buffer[20];
//	const uint8_t count = snprintf(buffer, 20, "L%02X", layer);
//	commandParser_sendReply(DC1, buffer, count);
//}
//
//

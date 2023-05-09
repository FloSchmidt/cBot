#include "robot_machine.h"
#include "arrayhelper.h"
#include "button.h"

#include <stdio.h>
#include <stddef.h>
#include <cBot.h>

extern button_t buttonDown;
int robotMachine_count = 0;


#define MAX_SPEED 12

void robotMachine_StateLoop_Idle(SM_StateMachine *self);
void robotMachine_StateLoop_IdlePressed(SM_StateMachine *self);
void robotMachine_StateLoop_Forward(SM_StateMachine *self);
void robotMachine_StateLoop_ForwardCrossing(SM_StateMachine *self);
void robotMachine_StateLoop_ForwardToLine(SM_StateMachine *self);
void robotMachine_StateEnter_ForwardToLine(SM_StateMachine *self);
void robotMachine_StateLoop_TurnLeft(SM_StateMachine *self);
void robotMachine_StateEnter_TurnLeft(SM_StateMachine *self);
void robotMachine_StateLoop_TurnRight(SM_StateMachine *self);
void robotMachine_StateEnter_TurnRight(SM_StateMachine *self);

SM_StateFktEntry stateFunctions[] = {
	{robotMachine_StateLoop_Idle, NULL, NULL },
	{robotMachine_StateLoop_IdlePressed, NULL, NULL },
	{robotMachine_StateLoop_Forward, NULL, NULL},
	{robotMachine_StateLoop_ForwardCrossing, NULL, NULL},
	{robotMachine_StateLoop_ForwardToLine, robotMachine_StateEnter_ForwardToLine, NULL},
	{robotMachine_StateLoop_TurnLeft, robotMachine_StateEnter_TurnLeft, NULL},
	{robotMachine_StateLoop_TurnRight, robotMachine_StateEnter_TurnRight, NULL},
};

enum State {
	IDLE,
	IDLE_PRESSED,
	FORWARD,
	FORWARD_CROSSING,
	FORWARD_TO_LINE,
	TURN_LEFT,
	TURN_RIGHT,
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
		robotMachine_count=1;
		stateMachine_transissionTo(self, FORWARD);
}

void robotMachine_StateLoop_Forward(SM_StateMachine *self)
{
	const int intensityL = getLightValue(SENSOR_LEFT);
	const int intensityR = getLightValue(SENSOR_RIGHT);

	int diff = intensityL - intensityR;

	if (diff < -100)
		setMotorRpm(0,MAX_SPEED);
	else if (diff > 100)
		setMotorRpm(MAX_SPEED,0);
	else
		setMotorRpm(MAX_SPEED, MAX_SPEED);

	if ((intensityL + intensityR) < 1800)
	{
		stateMachine_transissionTo(self, FORWARD_CROSSING);
	}
}

void robotMachine_StateLoop_ForwardCrossing(SM_StateMachine *self)
{
	const int intensityL = getLightValue(SENSOR_LEFT);
	const int intensityR = getLightValue(SENSOR_RIGHT);
	setMotorRpm(MAX_SPEED/2, MAX_SPEED);
	if ((intensityL + intensityR) > 2000)
	{
		if(--robotMachine_count)
		{
			stateMachine_transissionTo(self, FORWARD);
		}
		else
		{
			stateMachine_transissionTo(self, FORWARD_TO_LINE);
		}
	}
}

void robotMachine_StateEnter_ForwardToLine(SM_StateMachine *self)
{
	driveStrait(0.035);
}

void robotMachine_StateLoop_ForwardToLine(SM_StateMachine *self)
{
	if(!isMoving())
		stateMachine_transissionTo(self, IDLE);
}

void robotMachine_StateEnter_TurnLeft(SM_StateMachine *self)
{
	turn(90);
}

void robotMachine_StateLoop_TurnLeft(SM_StateMachine *self)
{
	if(!isMoving())
		stateMachine_transissionTo(self, IDLE);
}

void robotMachine_StateEnter_TurnRight(SM_StateMachine *self)
{
	turn(-90);
}

void robotMachine_StateLoop_TurnRight(SM_StateMachine *self)
{
	if(!isMoving())
		stateMachine_transissionTo(self, IDLE);
}

int robotMachine_EVENT_Forward(int count)
{
	if (robotMachine.currentState != IDLE)
		return 1;

	robotMachine_count = count;
	stateMachine_transissionTo(&robotMachine, FORWARD);
	return 0;
}

int robotMachine_EVENT_Right(int count)
{
	if (robotMachine.currentState != IDLE)
		return 1;

	stateMachine_transissionTo(&robotMachine, TURN_RIGHT);
	return 0;
}

int robotMachine_EVENT_Left(int count)
{
	if (robotMachine.currentState != IDLE)
		return 1;

	stateMachine_transissionTo(&robotMachine, TURN_LEFT);
	return 0;
}

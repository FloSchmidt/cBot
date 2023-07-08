#pragma once
#include "statemachine.h"
#include <stdint.h>

void robotMachine_Init(void);
void robotMachine_Task(void);

int robotMachine_EVENT_Forward(int count);
int robotMachine_EVENT_Left();
int robotMachine_EVENT_Right();


#pragma once
#include "statemachine.h"
#include <stdint.h>

void dispenserMachine_Init(void);
void dispenserMachine_Task(void);

//void dispenserMachine_EVENTcancel(void);
//uint8_t dispenserMachine_EVENTopenCompartment(uint8_t column, uint8_t layer);
//uint8_t dispenserMachine_EVENTenableFreeRotation(void);
//uint8_t dispenserMachine_EVENTdisableFreeRotation(void);

int robotMachine_EVENT_Forward(int count);


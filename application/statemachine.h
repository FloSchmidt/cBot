#pragma once
#include <stdint.h>

typedef struct SM_StateMachine
{
	uint8_t currentState;
	const struct SM_StateMachineConst *constData;
} SM_StateMachine;

typedef struct SM_StateMachineConst
{
	const uint8_t stateCount;
	const struct SM_StateFktEntry *stateMap;
	const uint8_t *name;
} SM_StateMachineConst;


// Generic state function signatures
typedef void (*SM_LoopFunc)(SM_StateMachine *self);
typedef void (*SM_EntryFunc)(SM_StateMachine *self);
typedef void (*SM_ExitFunc)(SM_StateMachine *self);

typedef struct SM_StateFktEntry
{
	SM_LoopFunc loopFn;
	SM_EntryFunc entryFn;
	SM_ExitFunc exitFn;
} SM_StateFktEntry;

void stateMachine_Init(SM_StateMachine *self);
void stateMachine_Task(SM_StateMachine *self);
void stateMachine_transissionTo(SM_StateMachine *self, uint8_t newState);

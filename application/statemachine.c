#define DEBUG_STATEMACHINE
#include "statemachine.h"
#include "sercom.h"

#include <assert.h>

extern sercom_t *serial;
#define COMMANDPARSER_SENDDEBUG(data, len, ...) \
    do { \
		uint8_t buffer[len]; \
		const uint8_t cnt = snprintf(buffer, len, data, ##__VA_ARGS__); \
		sercom_transmitStr(serial, buffer); \
    } while(0)

void stateMachine_Init(SM_StateMachine *self)
{
	
}

void stateMachine_Task(SM_StateMachine *self)
{
	SM_LoopFunc loop = self->constData->stateMap[self->currentState].loopFn;
	if (loop)
		loop(self);
}

void stateMachine_transissionTo(SM_StateMachine *self, uint8_t newState) 
{
	assert(newState < self->constData->stateCount);

#ifdef DEBUG_STATEMACHINE
	COMMANDPARSER_SENDDEBUG("%s: %d -> %d\n", 30, self->constData->name, self->currentState, newState);
#endif
	
	SM_ExitFunc exitFn = self->constData->stateMap[self->currentState].exitFn;
	if (exitFn)
		exitFn(self);
	
	self->currentState = newState;
	
	SM_EntryFunc entryFn = self->constData->stateMap[newState].entryFn;
	if (entryFn)
		entryFn(self);
}

#pragma once
#ifndef STATE_H_
#define STATE_H_

#include <stdbool.h>
#include "eurovision.h"
#include "map.h"


struct state_t;
typedef unsigned int* stateKey;
typedef char* stateData;

typedef struct state_t *State;

//enum for the add/remove vote functions - serves as a flag
typedef enum removeOrAddVote_t
{
	ADD_VOTE,
	REMOVE_VOTE
} removeOrAddVote;

//Gets an Id number, a state name and a song name
//returns a new State object with the given data 
State stateCreate(int id, const char* stateName, const char* songName);

//Gets a state 
//frees all its memory and clears it up
void stateDestroy(State state);

//Gets a state
//returns a new state which is a copy of the given one (new memory allocation)
State stateCopy(State state);

//gets a string and returns if Eurovision_SUCCESS if it's a valid state name
EurovisionResult isValidName(const char* name);

//gets a string and returns if Eurovision_SUCCESS if it's a valid state ID
EurovisionResult isValidId(const int id);

//gets a state and returns the votesGiven map object of the state
Map getVotesGiven(State state);

//gets a state and returns the PointsRecieved map object of the state
Map getPointsReceived(State state);

//gets the stateId of the given state
int getStateId(State state);

//gets a state, returns the state name
char * getStateName(State state);


// adds/removes 1 vote to receiverId state from the given state.
void updateVotesGiven(State state, int receiverId,removeOrAddVote flag);

#endif //STATE_H_

#include "state.h"
#include <stdlib.h>
#include <string.h>
#include "eurovision.h"
#include "map.h"
#include "helper.h"


//implementing the struct
struct state_t {
	int state_id;
	char* state_name; //Insert function to make sure its all lowercase
	char* song_name; // Insert function to make sure its all lowercase

	 //went for maps eventually bc the stateId doesn't have to be a zero-based index
	Map votes_given; // will keep track of the number of votes this country gave to a different country 
	Map points_recieved; // will keep track of the points each country gave to this state 

};

 State stateCreate(int id, const char* stateName, const char* songName)
 {
	 State state = malloc(sizeof(*state));
	 char* namePtr = malloc(strlen(stateName)+1);//Null terminator
	 char* songPtr = malloc(strlen(songName)+1);//Null terminator
	 
 	if (state == NULL || namePtr == NULL || songPtr == NULL)
 	{
		free(state);
		free(namePtr);
		free(songPtr);
		return NULL;
 	}
	 state->state_name = strcpy(namePtr,stateName);
	 state->song_name = strcpy(songPtr,songName);
	 state->state_id = id;
	 state->votes_given = mapCreate(copyInt,copyInt,freeInt,freeInt,compareInt);
	 state->points_recieved = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
	 return state;
 }

void stateDestroy(State state)
{
	if (state == NULL)
		return;

	free(state->song_name);
	free(state->state_name);
	mapDestroy(state->votes_given);
	mapDestroy(state->points_recieved);
	free(state);
}

State stateCopy(State state)
{
	State copy = stateCreate(state->state_id, state->state_name, state->song_name);
	if (copy == NULL)
		return NULL;
	return copy;
}
//////////////// Map functions: VotesGiven & pointsReceived 

//get functions:
Map getVotesGiven(State state)
 {
	if (state == NULL) return NULL;
	return state->votes_given;
 }

Map getPointsReceived(State state)
{
	if (state == NULL) return NULL;
	return state->points_recieved;
}

int* getStateIdPtr(State state)
 {
	if (state == NULL) return NULL;
	return &state->state_id;
 }
char * getStateName(State state)
 {
	return state->state_name;
 }

int getSizeofState()
 {
	State dummyState = stateCreate(1, "dummy", "dummy");
	int size=  sizeof(*dummyState);
	stateDestroy(dummyState);
	return size;
 }

//todo move to eurovision.c and change to static : setPointsReceivedStateToState
MapResult setPointsReceivedStateToState(State curState, State givingState, int points)
 {
	int* givingIdPtr = &(givingState->state_id);
	int* pointsPtr = &points;
	return mapPut(curState->points_recieved, givingIdPtr, pointsPtr);
 }


void updateVotesGiven(State state, int receiverId,removeOrAddVote flag)
 {
	if(state== NULL) return;

	int* votes = mapGet(state->votes_given, &receiverId);

	if (votes == NULL) //if receiver state doesn't exist
	{
		int first_vote = 1;
		mapPut(state->votes_given, &receiverId, &first_vote);
		return;
	}

	//do nothing if the giving state didn't vote for the receiving state 
	if (flag == REMOVE_VOTE && (*votes) == 0)
		return;

	(flag==ADD_VOTE) ? (*votes)++ :(*votes)--; // adding/removing 1 vote 
 }

/// Is valid - Id & Name ///////////////////////////////
EurovisionResult isValidId(const int id)
{
	if (id < 0)
		return EUROVISION_INVALID_ID;

	return EUROVISION_SUCCESS;
}

EurovisionResult isValidName(const char* name)
 {
	 int i = 0;
	 while(*(name + i)!= '\0')
	 {
		 if ((*(name + i) - 'a' < 0 || *(name + i) - 'z' > 0) && *(name + i) != ' ')
			 return EUROVISION_INVALID_NAME;
		 i++;
	 }
	 return EUROVISION_SUCCESS;
 }



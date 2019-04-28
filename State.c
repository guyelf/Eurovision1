#include "state.h"
#include <stdlib.h>
#include <string.h>
#include "eurovision.h"
#include "map.h"


//implementing the struct
struct state_t {
	int stateId;
	char* stateName; //Insert function to make sure its all lowercase
	char* songName; // Insert function to make sure its all lowercase

	 //went for maps eventually bc the stateId doesn't have to be a zero-based index
	Map votesGiven; // will keep track of the number of votes this country gave to a different country 
	Map pointsRecieved; // will keep track of the points each country gave to this state 

};

 ////////////////// compare/copy/free functions for a Map of Integers ///////////////////////////////////

 static MapKeyElement copyInt(MapKeyElement e) {
	 int *newInt = malloc(sizeof(int));
	 if (newInt == NULL) return NULL;
	 *newInt = *(int *)e;
	 return newInt;
 }

 static void freeInt(MapKeyElement e) {
	 free(e);
 }

 static int compareInt(MapKeyElement a, MapKeyElement b) {
	 return *(int *)a - *(int *)b;
 }
 /////////////////////////////////////////////////////////////////////////////
 State stateCreate(int id, const char* stateName, const char* songName)
 {
	 State state = malloc(sizeof(*state));
	 char* namePtr = malloc(strlen(stateName)+1);//Null terminator
	 char* songPtr = malloc(strlen(songName)+1);//Null terminator
	 
 	if (state == NULL || namePtr == NULL || songPtr == NULL)
		 return NULL;

	 //coping the names by value
	int i = 0;
	 while(*(namePtr+i)!= '\0')
	 {
		 *(namePtr + i) = *(stateName + i);
		 i++;
	 }
	 *(namePtr + i) = *(stateName + i);//adding the null terminating character
	 i = 0;
	 while (*(songPtr+i) != '\0')
	 {
		 *(songPtr + i) = *(songName + i);
		 i++;	 
	 }
	 *(songPtr + i) = *(songName + i);

	 state->stateName = namePtr;
	 state->songName = songPtr;
	 state->stateId = id;
	 state->votesGiven = mapCreate(copyInt,copyInt,freeInt,freeInt,compareInt);
	 state->pointsRecieved = mapCreate(copyInt, copyInt, freeInt, freeInt, compareInt);
	 return state;
 }

void stateDestroy(State state)
{
	if (state == NULL)
		return;

	free(state->songName);
	free(state->stateName);
	mapDestroy(state->votesGiven);
	mapDestroy(state->pointsRecieved);
	free(state);
}

State stateCopy(State state)
{
	State copy = stateCreate(state->stateId, state->stateName, state->songName);
	if (copy == NULL)
		return NULL;
	return copy;
}
//////////////// Map functions: VotesGiven & pointsReceived 

//get functions:
Map getVotesGiven(State state)
 {
	if (state == NULL) return NULL;
	return state->votesGiven;
 }

Map getPointsReceived(State state)
{
	if (state == NULL) return NULL;
	return state->pointsRecieved;
}

int* getStateIdPtr(State state)
 {
	if (state == NULL) return NULL;
	return &state->stateId;
 }
char * getStateName(State state)
 {
	return state->stateName;
 }

int getSizeofState()
 {
	State dummyState = stateCreate(1, "dummy", "dummy");
	int size=  sizeof(*dummyState);
	stateDestroy(dummyState);
	return size;
 }

MapResult setPointsReceivedStateToState(State curState, State givingState, int points)
 {
	int* givingIdPtr = &givingState->stateId;
	int* pointsPtr = &points;
	return mapPut(curState->pointsRecieved, givingIdPtr, pointsPtr);
 }


void updateVotesGiven(State state, int receiverId,removeOrAddVote flag)
 {
	if(state== NULL) return;

	int* votes = mapGet(state->votesGiven, &receiverId);
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
	 while((name+i)!= '\0')
	 {
		 if ((*(name + i) - 'a' < 0 || *(name + i) - 'z' > 0) && *(name + i) != ' ')
			 return EUROVISION_INVALID_NAME;
		 i++;
	 }
	 return EUROVISION_SUCCESS;
 }
////////////////////////////////////////////////////////





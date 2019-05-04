#include "state.h"
#include <stdlib.h>
#include <string.h>
#include "eurovision.h"
#include "map.h"
#include "helper.h"


//implementing the struct
struct state_t {
	int stateId;
	char* stateName; //Insert function to make sure its all lowercase
	char* songName; // Insert function to make sure its all lowercase
   
    //keeps track of the num votes  
    //this state gave to all other states  
	Map votesGiven;

    //keeps track of the points 
    //each state gave to this state 
	Map pointsRecieved;

};

 State stateCreate(int id, const char* stateName, const char* songName)
 {
	 if (stateName == NULL || songName == NULL)
		 return NULL;

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
	 state->stateName = strcpy(namePtr,stateName);
	 state->songName = strcpy(songPtr,songName);
	 state->stateId = id;
	 state->votesGiven = mapCreate(copyInt,copyInt,freeInt,freeInt,compareInt);
	 state->pointsRecieved = mapCreate(copyInt, copyInt, freeInt, 
                                       freeInt, compareInt);

	 if(state->votesGiven == NULL || state->pointsRecieved == NULL)
	 {
		 stateDestroy(state);
		 return NULL;
	 }
	 return state;
 }

void stateDestroy(State state)
{
	if (state == NULL) return;

	free(state->songName);
	free(state->stateName);
	mapDestroy(state->votesGiven);
	mapDestroy(state->pointsRecieved);
	free(state);
}

State stateCopy(State state)
{
	State copy = stateCreate(state->stateId, state->stateName, state->songName);
	if (copy == NULL) return NULL;
	return copy;
}
//////////////// Map functions: VotesGiven & pointsReceived 

//get functions:

//return the votesGiven object associated with this state
Map getVotesGiven(State state)
 {
	if (state == NULL) return NULL;
	return state->votesGiven;
 }

//return the PointsReceived object associated with this state
Map getPointsReceived(State state)
{
	if (state == NULL) return NULL;
	return state->pointsRecieved;
}

int getStateId(State state)
 {
	return state->stateId;
 }
char * getStateName(State state)
 {
	return state->stateName;
 }
/// Is valid - Id & Name ///////////////////////////////
EurovisionResult isValidId(const int id)
{
	if (id < 0) return EUROVISION_INVALID_ID;
	return EUROVISION_SUCCESS;
}

EurovisionResult isValidName(const char* name)
 {
	 int i = 0;
	 while(*(name + i)!= '\0')
	 {
         if ((*(name + i) - 'a' < 0 || *(name + i) - 'z' > 0) &&
             *(name + i) != ' ')
         {
             return EUROVISION_INVALID_NAME;
         }
		 i++;
	 }
	 return EUROVISION_SUCCESS;
 }
////////////////////////////////////////////////
void updateVotesGiven(State state, int receiverId, removeOrAddVote flag)
{
    if (state == NULL) return;

    int* votes = mapGet(state->votesGiven, &receiverId);

    if (votes == NULL) //if receiver state doesn't exist
    {
        int firstVote = 1;
        mapPut(state->votesGiven, &receiverId, &firstVote);
        return;
    }

    //do nothing if the giving state didn't vote for the receiving state 
    if (flag == REMOVE_VOTE && (*votes) == 0)return;

    (flag == ADD_VOTE) ? (*votes)++ : (*votes)--; // adding/removing 1 vote 
}

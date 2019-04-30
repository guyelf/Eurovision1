#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "map.h"
#include "eurovision.h"
#include "state.h"
#include "judge.h"
#include "helper.h"
#include <assert.h>
#include <string.h>

#define FIRST 1
#define INIT_SIZE 20
#define MAX_POINT 12

struct eurovision_t
{
	Map states;
	Map judges;

};

////////////////////////////// state - map - related functions /////////////////////////////////////////

//compare/copy/free functions for map keyElement (= unsigned int)
static MapKeyElement copyInt(MapKeyElement e) {
	unsigned int *newInt = malloc(sizeof(unsigned int));
	if (newInt == NULL) return NULL;
	*newInt = *(unsigned int *)e;
	return newInt;
}

static void freeInt(MapKeyElement e) {
	free(e);
}

static int compareInt(MapKeyElement a, MapKeyElement b) {
	return *(unsigned int *)a - *(unsigned int *)b;
}
///////////////

//Map State related functions
static MapDataElement copyStateData(MapDataElement data)
{
	if (data == NULL) return NULL;
	State newData = malloc(getSizeofState());
	if (newData == NULL) return NULL;

	newData = stateCopy((State) data);
	return newData;
}

static void freeStateData(MapDataElement data)
{
	stateDestroy((State)data);
}
/// Map Judge related functions
static MapDataElement copyJudgeData(MapDataElement judgeData) {
	if (judgeData == NULL) return NULL;

	Judge copyJudge = malloc(getJudgeSize());
	if (copyJudge == NULL) return NULL;

	copyJudge = judgeCopy((Judge)judgeData);
	return copyJudge;
}

static void freeJudgeData(MapDataElement data) {
	if (data == NULL)return;
	judgeDestroy((Judge)data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Eurovision eurovisionCreate()
{
	Map states = mapCreate(copyStateData, copyInt, freeStateData, freeInt, compareInt);
	Map judges = mapCreate(copyJudgeData, copyInt, freeJudgeData, freeInt, compareInt);

	Eurovision eurovision = (Eurovision) malloc(sizeof(*eurovision));

	if (eurovision == NULL || states == NULL || judges == NULL)
		return NULL;

	eurovision->judges = judges;
	eurovision->states = states;
	return eurovision;
}

void eurovisionDestroy(Eurovision eurovision)
{
	if (eurovision == NULL) return;

	mapDestroy(eurovision->states);
	mapDestroy(eurovision->judges);
	free(eurovision);
}

EurovisionResult eurovisionAddState(Eurovision eurovision, int stateId, const char* stateName, const char* songName)
{
	if (eurovision == NULL || stateName == NULL || songName == NULL)
		return EUROVISION_NULL_ARGUMENT;
	if (isValidId(stateId) != EUROVISION_SUCCESS)
		return EUROVISION_INVALID_ID;
	if (isValidName(stateName) != EUROVISION_SUCCESS || isValidName(songName) != EUROVISION_SUCCESS)
		return EUROVISION_INVALID_NAME;
	if (mapGet(eurovision->states, &stateId) != NULL)
		return EUROVISION_STATE_ALREADY_EXIST;
	
	State newState = stateCreate(stateId, stateName, songName);

	MapResult mapResult = mapPut(eurovision->states, &stateId, newState);
	if (mapResult != MAP_SUCCESS)
		return EUROVISION_OUT_OF_MEMORY;

	return EUROVISION_SUCCESS;
}

//checks  all the pre-requirements for running the add&remove votes
//returns the according EurovisionResult status
static EurovisionResult checkValidState_help(Eurovision eurovision, int stateGiver, int stateTaker)
{
	if (eurovision == NULL) return EUROVISION_NULL_ARGUMENT;
	if (isValidId(stateGiver) != EUROVISION_SUCCESS || isValidId(stateTaker) != EUROVISION_SUCCESS) return EUROVISION_INVALID_ID;

	State givingState = mapGet(eurovision->states, &stateGiver);
	State receivingState = mapGet(eurovision->states, &stateTaker);

	if (givingState == NULL || receivingState == NULL) return EUROVISION_STATE_NOT_EXIST;
	if (stateGiver == stateTaker) return EUROVISION_SAME_STATE;

	return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionAddVote(Eurovision eurovision, int stateGiver, int stateTaker)
{
	EurovisionResult result = checkValidState_help(eurovision, stateGiver, stateTaker);
	if (result != EUROVISION_SUCCESS) return result;

	State givingState = mapGet(eurovision->states, &stateGiver);
	updateVotesGiven(givingState, stateTaker,ADD_VOTE);

	return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionRemoveVote(Eurovision eurovision, int stateGiver, int stateTaker)
{
	EurovisionResult result = checkValidState_help(eurovision, stateGiver, stateTaker);
	if (result != EUROVISION_SUCCESS) return result;

	State givingState = mapGet(eurovision->states, &stateGiver);
	updateVotesGiven(givingState, stateTaker, REMOVE_VOTE);

	return EUROVISION_SUCCESS;
}



EurovisionResult eurovisionRemoveState(Eurovision eurovision, int stateId)
{
	if (eurovision == NULL) return EUROVISION_NULL_ARGUMENT;
	if (stateId < 0) return EUROVISION_INVALID_ID;
	if(mapGet(eurovision->states, &stateId) == NULL) return EUROVISION_STATE_NOT_EXIST;
	
	MapKeyElement stateIterator = mapGetFirst(eurovision->states);
	MapResult temp_result;
	//remove the stateId at all the votesGiven mapObjects of the other states
	while (stateIterator != NULL)
	{
		State curState = mapGet(eurovision->states, stateIterator);

		//removes the state at votesGiven MAP ! and checks that everything is good
		temp_result = mapRemove(getVotesGiven(curState), &stateId);
		if (temp_result != MAP_SUCCESS && temp_result != MAP_ITEM_DOES_NOT_EXIST)
			return EUROVISION_INVALID_ID;//any error

		stateIterator = mapGetNext(eurovision->states);
	}
	//sets 0 at pointsReceived in stateId at every other state
	stateIterator = mapGetFirst(eurovision->states);
	while (stateIterator != NULL)
	{
		State curState = mapGet(eurovision->states, stateIterator);
		temp_result = mapRemove(getPointsReceived(curState), &stateId);
		if (temp_result != MAP_SUCCESS && temp_result != MAP_ITEM_DOES_NOT_EXIST)
			return EUROVISION_INVALID_ID;

		stateIterator = mapGetNext(eurovision->states);
	}
	//handles the judge removal section
	MapKeyElement judgeIterator = mapGetFirst(eurovision->judges);
	while (judgeIterator != NULL)
	{
		Judge judge = mapGet(eurovision->judges, judgeIterator);
		int *votes = getJudgeVotes(judge);
		for (int i = 0; i < VotesNum; ++i)
		{
			if (votes[i] == stateId)
			{
				judgeDestroy(judge);
				break;
			}
		}
		judgeIterator = mapGetNext(eurovision->judges);
	}
	//removes the state completely of the euroviosion
	mapRemove(eurovision->states, &stateId);

	return EUROVISION_SUCCESS;
}
/////////////////////// Eurovision Judges ///////////////////////////////////////////////////////////////

EurovisionResult eurovisionAddJudge(Eurovision eurovision, int judgeId,
	const char* judgeName, int *judgeResults) {
	if (eurovision == NULL || judgeName == NULL || judgeResults == NULL) {
		return EUROVISION_NULL_ARGUMENT;
	}

	//check judgeId validity
	if (judgeId < 0) return EUROVISION_INVALID_ID;
	for (int i = 0; i < VotesNum; ++i) {
		if (judgeResults[i] < 0) return EUROVISION_INVALID_ID;
	}

	//check if judgeId is already exist
	if (mapContains(eurovision->judges, &judgeId)) {
		return EUROVISION_JUDGE_ALREADY_EXIST;
	}

	//check if all the states in the judgeResults are exist
	//and if the judge didn't vote more hen once for the same state
	//and if the states id are valid
	for (int i = 0; i < VotesNum; ++i) {
		if (!mapContains(eurovision->states, &judgeResults[i])) {
			return EUROVISION_STATE_NOT_EXIST;
		}

		if (judgeResults[i] < 0) return EUROVISION_INVALID_ID;

		for (int j=0; j<VotesNum; ++j){
		    if ((judgeResults[i] == judgeResults[j]) && (i != j)){
		        return EUROVISION_INVALID_ID;
		    }
		}
	}

	//check judgeName validity
	int i = 0;
	while (*(judgeName + i) != '\0') {
		if ((*(judgeName + i) < 'a' || *(judgeName + i) > 'z' ) && *(judgeName + i) != ' ')
			return EUROVISION_INVALID_NAME;
		i++;
	}

	Judge newJudge = judgeCreate(judgeId, judgeName, judgeResults);
	mapPut(eurovision->judges, &judgeId, newJudge);

	return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionRemoveJudge(Eurovision eurovision, int judgeId) {
	//check judgeId validity
	if (judgeId < 0) return EUROVISION_INVALID_ID;
	Judge judge = mapGet(eurovision->judges, &judgeId);
	int *judgeResults = getJudgeVotes(judge);
	for (int i = 0; i < VotesNum; ++i) {
		if (judgeResults[i] < 0) {
			return EUROVISION_INVALID_ID;
		};
	}
	//check if judge is exist
	if (!mapContains(eurovision->judges, &judgeId)) {
		return EUROVISION_JUDGE_NOT_EXIST;
	}

	mapRemove(eurovision->judges, &judgeId);
	return EUROVISION_SUCCESS;
}
///////////////////////////////////////////// End Judges /////////////////////////////////////////////////////



//that's an inner function that uses actual numbers bc it translates the rank given
//to the points system in the eurovision (no point for a #define for each rank)
static int getPointsFromRank(int rank)
{
	if (rank > 10 || rank < 1) return 0;
	if (rank == 1) return 12;
	if (rank == 2) return 10;
	return 11 - rank;
}

//gets the eurovision system, a giving state and a taking state
//returns the number of points the giving state gave to the taking state (based on the amount of votes)
static int getPointsFromState(Eurovision eurovision, State givingState, MapKeyElement takingState)
{
	//inner function
	assert(eurovision != NULL && givingState != NULL && takingState == NULL);
	if (!mapContains(getVotesGiven(givingState), takingState)) return 0;
	
	int takerVotes = *(int*) mapGet(getVotesGiven(givingState), takingState);
	MapKeyElement voteIterator = mapGetFirst(getVotesGiven(givingState));
	int rank = FIRST;
	while(voteIterator != NULL)
	{
		int curVotes = *(int*) mapGet(getVotesGiven(givingState), voteIterator);
		if (takerVotes < curVotes)
			rank++;//losing
		voteIterator = mapGetNext(getVotesGiven(givingState));
	}
	return getPointsFromRank(rank);
}



static MapResult setPointsReceivedState(Eurovision eurovision, State state)
{
	//inner function
	assert(eurovision != NULL && state != NULL);

	MapKeyElement stateIdPtr = getStateIdPtr(state);

	MapKeyElement stateIterator = mapGetFirst(eurovision->states);
	while(stateIterator != NULL)
	{
		State curState = mapGet(eurovision->states, stateIterator);
		int points = getPointsFromState(eurovision, curState, stateIdPtr);
		MapResult status = setPointsReceivedStateToState(state, curState, points);
		if( status != MAP_SUCCESS) 
			return status;
		stateIterator = mapGetNext(eurovision->states);
	}
	return MAP_SUCCESS;
}

//scans all the states in the eurovision and applies them with their points 
//(that they received from other states)

static MapResult setPointsReceived(Eurovision eurovision)
{
	//inner function
	assert(eurovision != NULL);

	MapKeyElement stateIterator = mapGetFirst(eurovision->states);
	
	while(stateIterator != NULL)
	{
		State curState = mapGet(eurovision->states, stateIterator);
		MapResult status = setPointsReceivedState(eurovision, curState);
		if (status != MAP_SUCCESS)
			return status;
		stateIterator = mapGetNext(eurovision->states);
	}
	return MAP_SUCCESS;
}

static int getAvgPointsReceived(State state)
{
	if (state == NULL) return -1;
	MapKeyElement pointsIterator = mapGetFirst(getPointsReceived(state));
	int count = 0, sum = 0;
	while (pointsIterator != NULL)
	{
		sum += *(int*)mapGet(getPointsReceived(state), pointsIterator);
		count++;
		pointsIterator = mapGetNext(getPointsReceived(state));
	}
	return (count == 0) ? 0 : sum / count;
}

//gets the avg points for a state from all the judges
static int getAvgPointsJudge(Eurovision eurovision, State state)
{
	MapKeyElement judgeIterator = mapGetFirst(eurovision->judges);
	int count = 0, sum = 0;
	while(judgeIterator != NULL)
	{
		Judge curJudge = mapGet(eurovision->judges, judgeIterator);
		int* votes = getJudgeVotes(curJudge);
		for (int i = 0; i < VotesNum; ++i)
		{
			if(votes[i] == *(getStateIdPtr(state)))
			{
				sum += getPointsFromRank(i);
				count++;
				break;
			}
		}

		judgeIterator = mapGetNext(eurovision->judges);
	}

	return (count == 0) ? 0 : (sum / count);
}



List eurovisionRunContest(Eurovision eurovision, int audiencePercent)
{
	if (eurovision == NULL || audiencePercent > 100 || audiencePercent < 0)
		return NULL;

	float* points = calloc(INIT_SIZE, sizeof(int));
	//string array for the states name
	char** ranks = malloc(INIT_SIZE);
	if (ranks == NULL || points == NULL) return NULL;

	if (setPointsReceived(eurovision) != MAP_SUCCESS) return NULL;

	MapKeyElement stateIterator = mapGetFirst(eurovision->states);
	int i = 0, times = 1;
	while(stateIterator != NULL)
	{
		State curState = mapGet(eurovision->states, stateIterator);
		if(i == times*INIT_SIZE-1)//realloc
		{
			points = realloc(points, INIT_SIZE);
			ranks = realloc(ranks, INIT_SIZE);
			if (ranks == NULL || points == NULL) 
				return NULL;//in case of an error
			times++;
		}		

		points[i] = (float)getAvgPointsReceived(curState)*((float)audiencePercent / 100) + (float)getAvgPointsJudge(eurovision, curState)*((100 - (float)audiencePercent) / 100);
		ranks[i] = getStateName(curState);// just assigning ptrs
		i++;
		stateIterator = mapGetNext(eurovision->states);
	}
	//sorts the points array and the ranks array according to that
	sortPointsAndRank(points, ranks, INIT_SIZE*times);

	List winners = listCreate(copyString, freeString);

	//if the list should be empty, returns empty list.
	if (points[0] == 0) return winners;

	//fill up the list
	listInsertFirst(winners, ranks[0]);
	for (i = 1; i < INIT_SIZE*times; i++)
	{
		listInsertAfterCurrent(winners, ranks[i]);
	}

	free(points);
	free(ranks);
	return winners;
}


List eurovisionRunAudienceFavorite(Eurovision eurovision)
{
	if (eurovision == NULL) return NULL;
	return eurovisionRunContest(eurovision, 100);
}

//TODO: function below
//List eurovisionRunGetFriendlyStates(Eurovision eurovision)
//{
//	if (eurovision == NULL) return NULL;
////means that the contest hasn't run yet
////checked if the first state has a pointsRecevied element or not
//	if (mapGetFirst(getPointsReceived(mapGetFirst(eurovision->states))) == NULL)
//		return NULL;
//
//	State curState = 
//}

/////////////////////////////////// FRIENDLY ///////////////////////////////////

//get id of potential friendly state who received 12 points from current state
static int getMaxVotedStateId(Map map){
	int max_voted_state_id = -1, MaxVotes=0;
	MAP_FOREACH(int *, i, map){
		int cur_state_votes = *(int *) mapGet(map, i);
		if (max_voted_state_id==-1 || cur_state_votes > MaxVotes){
			max_voted_state_id = *i;
			MaxVotes= cur_state_votes;
		}
	}
	return max_voted_state_id;
}

//get a concat string of the two friendly states
static char* getConcatStatesName (Map mapStates, int state_1_id, int state_2_id){
	State state_1 = mapGet(mapStates, &state_1_id);
	State state_2 = mapGet(mapStates, &state_2_id);

	char* state_1_name = getStateName(state_1);
	char* state_2_name = getStateName(state_2);
	char* new_friendly_states= malloc(strlen(state_1_name)+strlen(state_2_name)+4);
	if (new_friendly_states == NULL) return NULL;
	if (strcmp(state_1_name, state_2_name) <= 0){
		new_friendly_states = strcat(state_1_name, " - ");
		return (strcat(new_friendly_states, state_2_name));
	}
	else{
		new_friendly_states = strcat(state_2_name, " - ");
		return (strcat(new_friendly_states, state_1_name));
	}
}

List eurovisionRunGetFriendlyStates(Eurovision eurovision){
	if (eurovision == NULL) return NULL;
	List friendly_states = listCreate((CopyListElement) copyString,(FreeListElement) free);
	if(friendly_states==NULL){
        eurovisionDestroy(eurovision);
	    return NULL;
	}

    //pointsReceived is not initialized until the contest runs
    if(mapGetFirst(mapGetFirst(eurovision->states)) == NULL){
        eurovisionRunAudienceFavorite(eurovision);
    }

	MAP_FOREACH(int *, i, eurovision->states){
		State cur_state = mapGet(eurovision->states, i);
		Map cur_state_votes_given = getVotesGiven(cur_state);
		Map cur_state_points_received = getPointsReceived(cur_state);

        // the state that will get 12 points from cur_state
		int max_voted_state_id = getMaxVotedStateId(cur_state_votes_given);

		//from the state that was max_voted
		int points_received = *(int*) mapGet(cur_state_points_received, &max_voted_state_id);
		if (points_received == MAX_POINT && max_voted_state_id > *i){
            char *name = getConcatStatesName(eurovision->states, *i,
                                             max_voted_state_id);
            if (name == NULL){
                eurovisionDestroy(eurovision);
                return NULL;
            }
            listInsertLast(friendly_states, name);
		}
	}

    if (listSort(friendly_states, (CompareListElements) strcmp) != LIST_SUCCESS){
       eurovisionDestroy(eurovision);
       return NULL;
    }

	return friendly_states;
}
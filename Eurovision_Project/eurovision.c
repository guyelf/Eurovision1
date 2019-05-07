#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "map.h"
#include "eurovision.h"
#include "state.h"
#include "judge.h"
#include "helper.h"
#include <string.h>

#define FIRST 0
#define MAX_POINTS 12

//percantage limit for audience percent
#define TOP_LIMIT 100
//percantage limit for audience percent
#define BOTTOM_LIMIT 1

//used to translate votes ranks to points
#define LIMIT 10
//adding 4 more bytes (only used in concatenating strings for friendlyStates)
#define FORMAT 4
//the format expected friendlyStates to be presented
#define FORMAT_STR " - "

struct eurovision_t
{
	Map states;
	Map judges;

};

////////////////////////////// state - map - related functions ///////////////

//Map State related functions
static MapDataElement copyStateData(MapDataElement data)
{
	if (data == NULL) return NULL;
	State newData = stateCopy((State)data);
	if (newData == NULL) return NULL;
	return newData;
}

static void freeStateData(MapDataElement data)
{
	stateDestroy((State)data);
}
/// Map Judge related functions
static MapDataElement copyJudgeData(MapDataElement judgeData) {
	if (judgeData == NULL) return NULL;

	Judge copyJudge = judgeCopy((Judge)judgeData);
	if (copyJudge == NULL) return NULL;
	return copyJudge;
}

static void freeJudgeData(MapDataElement data) {
	judgeDestroy((Judge)data);
}

//////////////////////////////////////////////////////////////////////////





///////// STATIC FUNCTION /////////////////////


////////////////STATIC: PointsReceived Part ////////////////////////////

//inner function that uses actual numbers bc it translates the rank given
//to the points system in the eurovision (no point for a #define for each rank)
static int getPointsFromRank(int rank)
{
    if (rank > LIMIT || rank < 0) return 0;
    if (rank == 0) return MAX_POINTS;
    if (rank == 1) return LIMIT;
    return LIMIT - rank;
}
//gets the eurovision system, a giving state and a taking state
//returns the number of points the giving state gave to the taking state 
//(based on the amount of votes)
//-1 in-case of an error
static int getPointsFromState(Eurovision eurovision, State givingState,
    MapKeyElement takingState) {
    if (eurovision == NULL && givingState == NULL &&
        takingState == NULL) return -1;

    //if not exists
    if (!mapContains(getVotesGiven(givingState), takingState)) return 0;
    //if it's the same state
    if (getStateId(givingState) == *(int *)takingState) return 0;

    int num_votes = *((int*)mapGet(getVotesGiven(givingState), takingState));

    //otherwise goes to calculate the amount of points which is not good
    if (num_votes == 0) return 0;

    //iterating through the votesGiven of the givingState
    //returns the amount of points the votes mean
    MapKeyElement voteIterator = mapGetFirst(getVotesGiven(givingState));
    int rank = FIRST;
    //at this point in know for sure that there are value at the current 
    //votesGiven and that it's not the same state giving and taking
    while (voteIterator != NULL)
    {
        int curVotes = *(int*)mapGet(getVotesGiven(givingState), voteIterator);
        if ((num_votes <= curVotes && (*(int*)voteIterator <*(int*)takingState))
            || (num_votes < curVotes)) {
            rank++;//losing
        }

        voteIterator = mapGetNext(getVotesGiven(givingState));
    }
    return getPointsFromRank(rank);
}

//sets the points given by the giving state in pointsReceived map of curState
static MapResult setPointsReceivedStateToState(State curState,
    State givingState, int points) {
    int givingIdPtr = getStateId(givingState);
    Map pointsRecieved = getPointsReceived(curState);
    return mapPut(pointsRecieved, &givingIdPtr, &points);
}

static MapResult setPointsReceivedState(Eurovision eurovision, State state)
{
    if (eurovision == NULL && state == NULL) return MAP_NULL_ARGUMENT;
    int stateId = getStateId(state);
    MapKeyElement stateIdPtr = &stateId;

    MAP_FOREACH(MapKeyElement, stateIterator, eurovision->states)
    {
        State curState = mapGet(eurovision->states, stateIterator);
        int points = getPointsFromState(eurovision, curState, stateIdPtr);

        MapResult status =
       setPointsReceivedStateToState(state,curState, points);

        if (status != MAP_SUCCESS) return status;
    }
    return MAP_SUCCESS;
}

//scans all the states in the eurovision and applies them with their points 
//(that they received from other states)

static MapResult setPointsReceived(Eurovision eurovision)
{
    if (eurovision == NULL) return MAP_NULL_ARGUMENT;

    MapKeyElement stateIterator = mapGetFirst(eurovision->states);

    while (stateIterator != NULL)
    {
        State curState = mapGet(eurovision->states, stateIterator);
        MapResult status = setPointsReceivedState(eurovision, curState);
        if (status != MAP_SUCCESS)
            return status;
        //sets the iterator right again (nested foreach on the same map)
        MAP_FOREACH(MapKeyElement, deepIter, eurovision->states)
        {
            if (*(int*)deepIter == *(int*)stateIterator) break;
        }
        stateIterator = mapGetNext(eurovision->states);
    }
    return MAP_SUCCESS;
}

////MAP_Foreaching through all the states and checks if their PR map size is ok
//static bool isPointsReceivedValid(Eurovision eurovision)
//{
//    int statesNum = mapGetSize(eurovision->states);
//    MAP_FOREACH(MapKeyElement, stateIter, eurovision->states)
//    {
//        State state = mapGet(eurovision->states, stateIter);
//        Map pr = getPointsReceived(state);
//        int mapPointsSize = mapGetSize(pr);
//        if (statesNum != mapPointsSize) return false;
//    }
//    return true;
//}
///////////////////////////////// end PR related functions /////////

// Gets avg points for a state based on the votes of the other states
static double getAvgPointsReceived(State state)
{
    if (state == NULL) return -1;
    MapKeyElement pointsIterator = mapGetFirst(getPointsReceived(state));
    int count = 0, sum = 0;
    while (pointsIterator != NULL)
    {
        sum += *((int*)mapGet(getPointsReceived(state), pointsIterator));
        count++;
        pointsIterator = mapGetNext(getPointsReceived(state));
    }
    if (count == 1) return 0;// if it's just one state, edge case
    return (count == 0) ? 0 : sum / (double)(count - 1);//minus current state
}

//gets the avg points for a state from all the judges
static double getAvgPointsJudge(Eurovision eurovision, State state) {
    int count = 0, sum = 0;
    MAP_FOREACH(MapKeyElement, judgeIterator, eurovision->judges)
    {
        Judge curJudge = mapGet(eurovision->judges, judgeIterator);
        int* votes = getJudgeVotes(curJudge);
        for (int i = 0; i < VOTESNUM; ++i)
        {
            if (*(votes + i) == getStateId(state))
            {
                sum += getPointsFromRank(i);
                break;
            }
        }
        count++;
    }
    return (count == 0) ? 0 : (sum / (double)count);
}

static List getResultList(Eurovision eurovision, Map map)
{
    List resultList = listCreate((CopyListElement)copyString,
        (FreeListElement)free);

    if (resultList == NULL || map == NULL || eurovision == NULL) {
        mapDestroy(map);
        eurovisionDestroy(eurovision);
        listDestroy(resultList);
        return NULL;
    }

    while (mapGetSize(map) > 0)
    {
        int maxStateId = -1;
        double  maxPoints = -1;
        MAP_FOREACH(MapKeyElement, curId, map)
        {
            double curPoints = *(double*)mapGet(map, curId);

            if(doubleCompare(maxPoints,curPoints) < 0)
            {
                maxPoints = curPoints;
                maxStateId = *((int*)curId);
            }
        }

        State topState = mapGet(eurovision->states, &maxStateId);

        listInsertLast(resultList, getStateName(topState));
        mapRemove(map, &maxStateId);
    }
    return resultList;
}


//checks  all the pre-requirements for running the add&remove votes
//returns the according EurovisionResult status
static EurovisionResult checkValidState_help(Eurovision eurovision,
    int stateGiver, int stateTaker)
{
    if (eurovision == NULL) return EUROVISION_NULL_ARGUMENT;
    if (isValidId(stateGiver) != EUROVISION_SUCCESS ||
        isValidId(stateTaker)!=EUROVISION_SUCCESS)return EUROVISION_INVALID_ID;

    State givingState = mapGet(eurovision->states, &stateGiver);
    State receivingState = mapGet(eurovision->states, &stateTaker);

    if (givingState == NULL ||
        receivingState == NULL) return EUROVISION_STATE_NOT_EXIST;
    if (stateGiver == stateTaker) return EUROVISION_SAME_STATE;

    return EUROVISION_SUCCESS;
}
/////////// STATIC:  FRIENDLY //////// //////////

//get id of potential friendly state who received 12 points from current state
//gets the votesGiven map
//returns the stateId with the maximal votes (12 points) 
//ERRORS: returns -1, there's an error with the votesGiven of the state
static int getMaxVotedStateId(Map votes) {
    int maxVotedStateId = -1;
    int MaxVotes = 0;//maxVotes has to be bigger than 0 in order to count
    MAP_FOREACH(MapKeyElement, votesKey, votes) {

        if (mapContains(votes, votesKey) == false) {
            continue;
        }

        int curStateVotes = *((int *)mapGet(votes, votesKey));
        if (curStateVotes > MaxVotes) {
            maxVotedStateId = *((int *)votesKey);
            MaxVotes = curStateVotes;
        }
    }
    return maxVotedStateId;
}

//get a concat string of the two friendly states
static char* getConcatStatesName(Map states, int stateId1, int stateId2) {

    State state1 = mapGet(states, &stateId1);
    State state2 = mapGet(states, &stateId2);
    if (state1 == NULL || state2 == NULL) return NULL;

    char* name1 = getStateName(state1);
    char* name2 = getStateName(state2);
    char* friendlyStates = malloc(strlen(name1) + strlen(name2) + FORMAT);

    if (friendlyStates == NULL) return NULL;

    if (strcmp(name1, name2) <= 0) {
        strcpy(friendlyStates, name1);//init newFriendlyStates with state1
        friendlyStates = strcat(friendlyStates, FORMAT_STR);// adds " - "
        return strcat(friendlyStates, name2);// adds state2
    }
    else {
        strcpy(friendlyStates, name2);
        friendlyStates = strcat(friendlyStates, FORMAT_STR);
        return strcat(friendlyStates, name1);
    }
}


//////////


Eurovision eurovisionCreate()
{
	Map states = mapCreate(copyStateData, copyInt, freeStateData, 
        freeInt, compareInt);
	Map judges = mapCreate(copyJudgeData, copyInt, freeJudgeData, 
        freeInt, compareInt);

	Eurovision eurovision = (Eurovision) malloc(sizeof(*eurovision));

	if (eurovision == NULL || states == NULL || judges == NULL)
	{
		mapDestroy(states);
		mapDestroy(judges);
		eurovisionDestroy(eurovision);
		return NULL;
	}
		
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

EurovisionResult eurovisionAddState(Eurovision eurovision, int stateId,
    const char* stateName, const char* songName)
{
	if (eurovision == NULL || stateName == NULL
        || songName == NULL) return EUROVISION_NULL_ARGUMENT;
	if (isValidId(stateId) != EUROVISION_SUCCESS) return EUROVISION_INVALID_ID;
    if (isValidName(stateName) != EUROVISION_SUCCESS ||
        isValidName(songName) != EUROVISION_SUCCESS) {
        return EUROVISION_INVALID_NAME;
    }

	if (mapGet(eurovision->states, &stateId) != NULL)
	{
        return EUROVISION_STATE_ALREADY_EXIST;
	}
	State newState = stateCreate(stateId, stateName, songName);
	MapResult mapResult = mapPut(eurovision->states, 
                       &stateId, newState);
	stateDestroy(newState);

	if (mapResult != MAP_SUCCESS)
	{
		eurovisionDestroy(eurovision);
		return EUROVISION_OUT_OF_MEMORY;
	}
	return EUROVISION_SUCCESS;
}


EurovisionResult eurovisionAddVote(Eurovision eurovision, int stateGiver,
                                     int stateTaker){
	EurovisionResult result = checkValidState_help(eurovision, stateGiver, 
                                                    stateTaker);
	if (result != EUROVISION_SUCCESS) return result;

	State givingState = mapGet(eurovision->states, &stateGiver);
	updateVotesGiven(givingState, stateTaker,ADD_VOTE);

	return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionRemoveVote(Eurovision eurovision, 
                                       int stateGiver, int stateTaker){
	EurovisionResult result = checkValidState_help(eurovision, stateGiver,
                                                    stateTaker);
	if (result != EUROVISION_SUCCESS) return result;

   
	State givingState = mapGet(eurovision->states, &stateGiver); 
	updateVotesGiven(givingState, stateTaker, REMOVE_VOTE);
 //to pass Idan's tests swap here between taker&giver
	return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionRemoveState(Eurovision eurovision, int stateId){
	if (eurovision == NULL) return EUROVISION_NULL_ARGUMENT;
	if (stateId < 0) return EUROVISION_INVALID_ID;

	if(mapContains(eurovision->states, &stateId)==false){
        return EUROVISION_STATE_NOT_EXIST;
	}
	MapResult status;
	//remove the stateId at all the votesGiven mapObjects of the other states
	MAP_FOREACH(MapKeyElement, stateIterator, eurovision->states)
	{
		State curState = mapGet(eurovision->states, stateIterator);

	//removes the state at votesGiven MAP ! and checks that everything is good 
		status = mapRemove(getVotesGiven(curState), &stateId);
		
	    if(status != MAP_SUCCESS && 
            status != MAP_ITEM_DOES_NOT_EXIST) return EUROVISION_INVALID_ID;
	}
	//sets 0 at pointsReceived in stateId at every other state
    MAP_FOREACH(MapKeyElement, stateIterator, eurovision->states)
	{
		State curState = mapGet(eurovision->states, stateIterator);
		status = mapRemove(getPointsReceived(curState), &stateId);
		
	    if(status != MAP_SUCCESS && 
            status != MAP_ITEM_DOES_NOT_EXIST) return EUROVISION_INVALID_ID;
	}
	//handles the judge removal section
    bool canRemove = true;
    while(canRemove){
        canRemove = false;
        MAP_FOREACH(MapKeyElement, judgeIterator, eurovision->judges)
        {
            Judge judge = mapGet(eurovision->judges, judgeIterator);
            int *votes = getJudgeVotes(judge);
            for (int i = 0; i < VOTESNUM; ++i) {
                if (votes[i] == stateId) {
                    int idToRemove = getJudgeId(judge);
                    mapRemove(eurovision->judges, &idToRemove);
                    canRemove = true;
                    break;
                }
            }
        }
    }
	//removes the state completely of the euroviosion
	mapRemove(eurovision->states, &stateId);
	return EUROVISION_SUCCESS;
}
/////////////////////// Eurovision Judges //////////////////////////////////

EurovisionResult eurovisionAddJudge(Eurovision eurovision, int judgeId,
	const char* judgeName, int *judgeResults) {
	if (eurovision == NULL || judgeName == NULL ||
        judgeResults == NULL) return EUROVISION_NULL_ARGUMENT;

	//check judgeId validity
	if (judgeId < 0) return EUROVISION_INVALID_ID;

	//check if judgeId is already exist}
    if (mapContains(eurovision->judges, &judgeId)) {
        return EUROVISION_JUDGE_ALREADY_EXIST;
    }
	//check if all the states in the judgeResults exist
	//and if the judge didn't vote more than once for the same state
	//and if the states id are valid
	for (int i = 0; i < VOTESNUM; ++i) 
    {
        if (judgeResults[i] < 0) return EUROVISION_INVALID_ID;

		if (!mapContains(eurovision->states, &judgeResults[i])) {
			return EUROVISION_STATE_NOT_EXIST;
		}

		for (int j=0; j<VOTESNUM; ++j){
		    if ((judgeResults[i] == judgeResults[j]) && (i != j)){
		        return EUROVISION_INVALID_ID;
		    }
		}
	}

	//check judgeName validity
    if(!isValidJudgeName(judgeName)) return EUROVISION_INVALID_NAME;

	Judge newJudge = judgeCreate(judgeId, judgeName, judgeResults);
	if (newJudge == NULL)
	{
		eurovisionDestroy(eurovision);
		return EUROVISION_OUT_OF_MEMORY;
	}
	MapResult status = mapPut(eurovision->judges, &judgeId,
                                                  newJudge);
	judgeDestroy(newJudge);
	if(status == MAP_OUT_OF_MEMORY)
	{
		eurovisionDestroy(eurovision);
		return EUROVISION_OUT_OF_MEMORY;
	}
	return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionRemoveJudge(Eurovision eurovision, int judgeId) {
	//check judgeId validity
	if (judgeId < 0) return EUROVISION_INVALID_ID;
	Judge judge = mapGet(eurovision->judges, &judgeId);
	if (judge == NULL) return EUROVISION_JUDGE_NOT_EXIST;
	int *judgeResults = getJudgeVotes(judge);
	for (int i = 0; i < VOTESNUM; ++i) {
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
///////////////////////////////////////////// End Judges /////////////////



List eurovisionRunContest(Eurovision eurovision, int audiencePercent)
{
	if (eurovision == NULL || audiencePercent > TOP_LIMIT ||
        audiencePercent < BOTTOM_LIMIT) return NULL;

	Map resultMap = mapCreate(copyDouble, copyInt, 
      (freeMapDataElements)free, (freeMapKeyElements)
                    free, compareInt);

    if(resultMap == NULL){
        eurovisionDestroy(eurovision);
        return NULL;
    }

    //assuring pointsReceived is ok before the iteration
    setPointsReceived(eurovision);

	MAP_FOREACH(MapKeyElement,stateId,eurovision->states)
	{
		State curState= mapGet(eurovision->states, stateId);

        double avgPointsRec = (double)getAvgPointsReceived(curState)*
                              ((double)audiencePercent / 100);

        double avgJudgePoints = (double)getAvgPointsJudge(eurovision, curState)*
            ((double)(100 - audiencePercent) / 100);

		double statePoints = avgPointsRec+avgJudgePoints;

		mapPut(resultMap, stateId, &statePoints);
	}

	List resultList = getResultList(eurovision, resultMap);
	mapDestroy(resultMap);
	return resultList;
}

List eurovisionRunAudienceFavorite(Eurovision eurovision)
{
	if (eurovision == NULL) return NULL;
	return eurovisionRunContest(eurovision, 100);
}

/////////////////////////////////// FRIENDLY ////////////////////////////

List eurovisionRunGetFriendlyStates(Eurovision eurovision){
	if (eurovision == NULL) return NULL;

	List friendly_states = listCreate((CopyListElement) 
                                       copyString,(FreeListElement) free);
	if(friendly_states==NULL){
        eurovisionDestroy(eurovision);
	    return NULL;
	}

	MapKeyElement firstKey = mapGetFirst(eurovision->states);
	if (firstKey == NULL || !mapContains(eurovision->states,firstKey)){
		listDestroy(friendly_states);
		return NULL;
	}
    //assuring pointsReceived is ok before the iteration
    setPointsReceived(eurovision);

	MAP_FOREACH(MapKeyElement, i, eurovision->states){
		State curState = mapGet(eurovision->states, i);
		Map curVotesGiven = getVotesGiven(curState);
		Map curPointsReceived = getPointsReceived(curState);

        // the state that will get 12 points from curState
		int maxVotedStateId = getMaxVotedStateId(curVotesGiven);
		//if it doesn't have a valid votesGiven map (no votes were given)
		if(maxVotedStateId == -1){
			continue;
		}
		//now checking if the points the current state got from the state
		//she nominated for 12 points is also 12 points
		int pointsReceived=*(int*)mapGet(curPointsReceived, &maxVotedStateId);
		if (pointsReceived == MAX_POINTS && (maxVotedStateId > *(int*)i)){
            char *name = getConcatStatesName(eurovision->states,
											*(int*)i,
                                            maxVotedStateId);
            if (name == NULL){
				eurovisionDestroy(eurovision);
				listDestroy(friendly_states);
				return NULL;
            }
			//sends a copy
            listInsertLast(friendly_states, name);
			free(name);
		}
	}
   if (listSort(friendly_states, (CompareListElements) strcmp) != LIST_SUCCESS){
		eurovisionDestroy(eurovision);
		listDestroy(friendly_states);
		return NULL;
    }
	return friendly_states;
}

#pragma once
#ifndef JUDGE_H_
#define JUDGE_H_
#define VOTESNUM 10
#include "map.h"

typedef struct judge_t * Judge;

//returns the sizeof jude in the memory (good for malloc and friends)
int getJudgeSize();

//returns the same pointer to the judgeVotes array
int* getJudgeVotes(Judge judge);

//gets an id,name, and array of votes
//returns a new judge with the assigned data
Judge judgeCreate(int judgeId,const char* judgeName, int* judgeVotes);

//gets a judge
//removes it completely
void judgeDestroy(Judge judge);

//gets a judge
//returns a new judge instance which is a copy of the given judge
Judge judgeCopy(Judge judge);

//checks if the given name for the judge is valid
bool isValidJudgeName(const char* name);

//return the id associated with the given judge
int getJudgeId(Judge judge);

//return the name associated with the given judge
char* getJudgeName(Judge judge);

#endif /* JUDGE_H_ */


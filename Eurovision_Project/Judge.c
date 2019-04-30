#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include "judge.h"

struct judge_t {
	int judge_id;
	char* judge_name;
	int* judge_votes;
};

int getJudgeSize()
{
	return sizeof(struct judge_t);
}

//returns the same pointer to the judgeVotes array
int* getJudgeVotes(Judge judge)
{
	return judge->judge_votes;
}

Judge judgeCreate(int judge_id,const char* judge_name, int* judge_votes) {
	if (judge_name == NULL || judge_votes == NULL) {
		return NULL;
	}

	Judge new_judge = malloc(sizeof(*new_judge));
	char* name_ptr = new_judge->judge_name;
	int* votes_ptr = new_judge->judge_votes;

	name_ptr = malloc(sizeof(strlen(judge_name) + 1));
	votes_ptr = calloc(VotesNum, sizeof(int));
	if(name_ptr == NULL || votes_ptr == NULL)
	{
		free(name_ptr);
		free(votes_ptr);
		return NULL;
	}
	//used memcpy instead - remove comment after succeeding tests
	//for (int i = 0; i < VotesNum; ++i) {
	//	votesPtr[i] = judge_votes[i];
	//}
	new_judge->judge_id = judge_id;
	new_judge->judge_name = strcpy(name_ptr, judge_name);
	new_judge->judge_votes = memcpy(votes_ptr,judge_votes,(sizeof(int)*VotesNum));

	return new_judge;
}

void judgeDestroy(Judge judge) {
	if (judge == NULL) return;

	free(judge->judge_name);
	free(judge->judge_votes);
	free(judge);
}

Judge judgeCopy(Judge judge) {
	Judge new_judge = malloc(sizeof(*new_judge));
	if (new_judge == NULL) return NULL;
	new_judge = judgeCreate(judge->judge_id, judge->judge_name, judge->judge_votes);
	return new_judge;
}

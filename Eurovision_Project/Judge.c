#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

	Judge newJudge = malloc(sizeof(*newJudge));
	char* namePtr = newJudge->judge_name;
	int* votesPtr = newJudge->judge_votes;

	namePtr = malloc(sizeof(strlen(judge_name) + 1));
	votesPtr = calloc(VotesNum, sizeof(int));

	int i = 0;
	while (*(judge_name + i) != '\0') {
		*(namePtr + i) = *(judge_name + i);
		i++;
	}
	*(namePtr + i) = *(judge_name + i);

	for (i = 0; i < VotesNum; ++i) {
		votesPtr[i] = judge_votes[i];
	}

	newJudge->judge_id = judge_id;
	newJudge->judge_name = namePtr;
	newJudge->judge_votes = votesPtr;

	return newJudge;
}

void judgeDestroy(Judge judge) {
	if (judge == NULL) return;

	free(judge->judge_name);
	free(judge->judge_votes);
	free(judge);
}

Judge judgeCopy(Judge judge) {
	Judge newJudge = malloc(sizeof(*newJudge));
	if (newJudge == NULL) return NULL;
	newJudge = judgeCreate(judge->judge_id, judge->judge_name, judge->judge_votes);
	return newJudge;
}

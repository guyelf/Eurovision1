#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include "judge.h"

struct judge_t {
	int judgeId;
	char* judgeName;
	int* judgeVotes;
};

int getJudgeSize()
{
	return sizeof(struct judge_t);
}

//returns the same pointer to the judgeVotes array
int* getJudgeVotes(Judge judge)
{
	return judge->judgeVotes;
}

Judge judgeCreate(int judgeId,const char* judgeName, int* judgeVotes) {
	if (judgeName == NULL || judgeVotes == NULL) {
		return NULL;
	}

	Judge newJudge = malloc(sizeof(*newJudge));
	char* namePtr = malloc(strlen(judgeName) + 1);
	int* votesPtr = calloc(VOTESNUM, sizeof(int));
	if(namePtr == NULL || votesPtr == NULL || newJudge == NULL)
	{
		free(newJudge);
		free(namePtr);
		free(votesPtr);
		return NULL;
	}
	newJudge->judgeId = judgeId;
	newJudge->judgeName = strcpy(namePtr, judgeName);
	newJudge->judgeVotes = memcpy(votesPtr,judgeVotes,(sizeof(int)*VOTESNUM));

	return newJudge;
}

void judgeDestroy(Judge judge) {
	if (judge == NULL) return;

	free(judge->judgeName);
	free(judge->judgeVotes);
	free(judge);
}

Judge judgeCopy(Judge judge) {
	if (judge == NULL) return NULL;
	Judge newJudge = judgeCreate(judge->judgeId, judge->judgeName,
                                 judge->judgeVotes);
	if (newJudge == NULL) return NULL;
	return newJudge;
}

bool isValidJudgeName(char* name)
{
    int i = 0;
    while (*(name + i) != '\0')
    {
        if ((*(name + i) < 'a' || *(name + i) > 'z') && *(name + i) != ' ')
            return false;
        i++;
    }
    return true;
}

int getJudgeId(Judge judge)
{
    return judge->judgeId;
}

char* getJudgeName(Judge judge)
{
    return judge->judgeName;
}


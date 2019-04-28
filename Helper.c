#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include <assert.h>

void swapInt(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void swapRanks(char** state1, char** state2)
{
	char* temp = *state1;
	*state1 = *state2;
	*state2 = temp;
}

static int indexOfMax(int * arr, int length)
{
	int i_max = 0;
	for (int i = 0; i < length; i++)
	{
		if (arr[i] > arr[i_max])
			i_max = i;
	}
	return i_max;
}

void sortPointsAndRank(int* points, char** ranks, int size)
{
	if (points == NULL || ranks == NULL) return;
	for (int i = size; i > 1; i--)
	{
		int i_max = indexOfMax(points, i);
		swapInt(&points[i - 1], &points[i_max]);
		swapRanks(&ranks[i-1], &ranks[i_max]);
	}
	//should swap both in the same way
}

ListElement copyString(ListElement str)
{
	assert(str);
	char* copy = malloc(strlen(str) + 1);
	return copy ? strcpy(copy, str) : NULL;	
}
void freeString(ListElement str)
{
	free(str);
}
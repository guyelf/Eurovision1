#include "helper.h"


void swapFloat(float* a, float* b)
{
	float temp = *a;
	*a = *b;
	*b = temp;
}

void swapRanks(char** state1, char** state2)
{
	char* temp = *state1;
	*state1 = *state2;
	*state2 = temp;
}

static int indexOfMax(float * arr, int length)
{
	int i_max = 0;
	for (int i = 0; i < length; i++)
	{
		if (arr[i] > arr[i_max])
			i_max = i;
	}
	return i_max;
}
//puts the max in the end of the array
void sortPointsAndRank(float* points, char** ranks, int size)
{
	if (points == NULL || ranks == NULL) return;
	for (int i = size; i > 1; i--)
	{
		int i_max = indexOfMax(points, i);
		swapFloat(&points[i - 1], &points[i_max]);
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

//compare/copy/free functions for map keyElement (= unsigned int)
MapKeyElement copyInt(MapKeyElement e) {
	int *newInt = malloc(sizeof(int));
	if (newInt == NULL) return NULL;
	*newInt = *(int *)e;
	return newInt;
}

void freeInt(MapKeyElement e) {
	free(e);
}

int compareInt(MapKeyElement a, MapKeyElement b) {
	return *(int *)a - *(int *)b;
}

MapKeyElement copyFloat(MapKeyElement e) {
	float *newFloat = malloc(sizeof(float));
	if (newFloat == NULL) return NULL;
	*newFloat = *(float *)e;
	return newFloat;
}
///////////////
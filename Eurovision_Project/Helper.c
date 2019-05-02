#include "helper.h"

//compare/copy/free functions for map 
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
 MapKeyElement copyDouble(MapKeyElement e) {
	double *newDouble = malloc(sizeof(double));
	if (newDouble == NULL) return NULL;
	*newDouble = *(double *)e;
	return newDouble;
}
ListElement copyString(ListElement str)
{
	if (str == NULL) return NULL;
	char* copy = malloc(strlen(str) + 1);
	if (copy==NULL) return NULL;
	return strcpy(copy, str);
}
void freeString(ListElement str)
{
	free(str);
}
///////////////
#include "helper.h"

//compare/copy/free functions for map 
MapKeyElement copyInt(MapKeyElement element) {
	int *newInt = malloc(sizeof(int));
	if (newInt == NULL) return NULL;
	*newInt = *(int *)element;
	return newInt;
}

void freeInt(MapKeyElement element) {
	free(element);
}

int compareInt(MapKeyElement num1, MapKeyElement num2) {
	return *(int *)num1 - *(int *)num2;
}

 MapKeyElement copyDouble(MapKeyElement element) {
	double *newDouble = malloc(sizeof(double));
	if (newDouble == NULL) return NULL;
	*newDouble = *(double *)element;
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
/////////////////////////////////////////////////
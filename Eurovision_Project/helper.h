#pragma once
#ifndef HELPER_H_
#define HELPER_H_
#pragma warning(disable:4996)
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include <assert.h>
#include "map.h"

//function for listCreate to copy the names of the states
ListElement copyString(ListElement str);			

//function for listCreate to free the string's ptrs
void freeString(ListElement str);

MapKeyElement copyDouble(MapKeyElement element);

int compareInt(MapKeyElement num1, MapKeyElement num2);

void freeInt(MapKeyElement element);

MapKeyElement copyInt(MapKeyElement element);


#endif //HELPER_H_
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

MapKeyElement copyDouble(MapKeyElement e);

int compareInt(MapKeyElement a, MapKeyElement b);

void freeInt(MapKeyElement e);

MapKeyElement copyInt(MapKeyElement e);


#endif //HELPER_H_
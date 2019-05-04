#pragma once
#ifndef HELPER_H_
#define HELPER_H_
#pragma warning(disable:4996)
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include <assert.h>
#include "map.h"
#define EPSILON 0.0000001

 

//function for listCreate to copy the names of the states
ListElement copyString(ListElement str);			

//function for listCreate to free the string's ptrs
void freeString(ListElement str);

//compare to double types 
//returns bigger than 0 if num1 is bigger than num2 
int doubleCompare(double num1, double num2);

//gets an element of type double returns a new double with the same data
MapKeyElement copyDouble(MapKeyElement element);

//compares to elements of type int
// bigger than 0 if num1 is bigger than num2 
int compareInt(MapKeyElement num1, MapKeyElement num2);

//frees an int ptr
void freeInt(MapKeyElement element);

//gets an element of type int returns a new int with the same data
MapKeyElement copyInt(MapKeyElement element);


#endif //HELPER_H_
#pragma once
#ifndef HELPER_H_
#define HELPER_H_

void swapInt(int* a, int* b);

//not entirely sure about the behavior of this swap
void swapRanks(char** state1, char** state2);

//sorts the points from high to low and according to that the ranks of the states
void sortPointsAndRank(int* points, char** ranks, int size);

//function for listCreate to copy the names of the states
ListElement copyString(ListElement str);			

//function for listCreate to free the string's ptrs
void freeString(ListElement str);










#endif HELPER_H_
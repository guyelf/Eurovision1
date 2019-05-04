#include <stdlib.h>
#include "map.h"

#include <stdbool.h>


////////////////// Internal functions (static) section /////////////////
/*!
* Macro for iterating over a map with a mapNode obj.
* Declares a new iterator for the loop.
*/
#define MAPNODE_FOREACH(iterator, map)\
    for(MapNode iterator = map->head->next; \
        iterator ;\
        iterator = iterator->next)


typedef struct map_node {
	MapKeyElement keyElement;
	MapDataElement dataElement;
	struct map_node *next;

} *MapNode;

//implementation of the MAP from the header file
struct Map_t {
	MapNode head;
	int size;
	MapNode iterator;

	copyMapDataElements copyMapData;
	copyMapKeyElements copyMapKey;
	freeMapDataElements freeMapData;
	freeMapKeyElements freeMapKey;
	compareMapKeyElements compare;
};


static MapNode mapNodeCreate(Map map, MapKeyElement keyElement,
                             MapDataElement dataElement) 
{
	if (keyElement == NULL || dataElement == NULL) return NULL;

	MapNode mapNode = malloc(sizeof(*mapNode));
	if (mapNode == NULL) return NULL;

	mapNode->dataElement = map->copyMapData(dataElement);
	if (mapNode->dataElement == NULL) {
		free(mapNode);
		return NULL;
	}

	mapNode->keyElement = map->copyMapKey(keyElement);
	if (mapNode->keyElement == NULL) {
		map->freeMapData(mapNode->dataElement);
		free(mapNode);
		return NULL;
	}

	mapNode->next = NULL;
	return mapNode;
}

static void mapNodeDestroy(Map map, MapNode mapNode) {
	map->freeMapData(mapNode->dataElement);
	map->freeMapKey(mapNode->keyElement);
	free(mapNode);

}
//gets a map and a key and returns the node associated with the key.
//NULL otherwise.
static MapNode mapNodeFindByKey(Map map, MapKeyElement key) {
	if (map == NULL || key == NULL || mapGetSize(map) == 0) return NULL;

	MAPNODE_FOREACH(iterator, map) 
    {
		if (map->compare(iterator->keyElement, key) == 0)
		{
            return iterator;
		}
              
	}
	return NULL;
}

//gets a map
//returns the first node in the map (not the dummy head)
static MapNode mapNodeGetFirst(Map map) {
	if (map == NULL) return NULL;
	return map->head->next;
}

//gets a map and a target node
//return the node located before the target in the map
// NULL otherwise.
static MapNode mapNodeGetPrev(Map map, MapNode target) {
	if (map == NULL || target == NULL) return NULL;

	MapNode node = mapNodeGetFirst(map);
	if (node->next == NULL || map->compare(node->keyElement,
                                            target->keyElement) == 0) 
    {
		return map->head;
	}

	while (node->next->next != NULL) {
		if (map->compare(node->next->keyElement, target->keyElement) == 0)
		{
            return node;
		}
		node = node->next;
	}
	//if the last one is the target
	if (map->compare(node->next->keyElement, target->keyElement) == 0)
	{
        return node;
	}
	//otherwise - error
	return NULL;
}
//updates the map to the correct status after an insertion happened successfully 
static MapResult mapUpdateInsertion(Map map)
{
	map->size++;
	map->iterator = NULL;
	return MAP_SUCCESS;
}

//gets a map and a node within the map
//updates the map to the correct status after a removal of node happened
static MapResult mapUpdateRemoval(Map map, MapNode node)
{
	mapNodeDestroy(map, node);
	map->iterator = NULL;
	map->size--;
	return MAP_SUCCESS;
}

/////////////////////////End static section ////////////////

///////////////////////////////// MAP part implementation ////////
Map mapCreate(copyMapDataElements copyDataElement,
	copyMapKeyElements copyKeyElement,
	freeMapDataElements freeDataElement,
	freeMapKeyElements freeKeyElement,
	compareMapKeyElements compareKeyElements) {

	if (copyKeyElement == NULL || copyDataElement == NULL ||
		freeDataElement == NULL ||
		freeKeyElement == NULL || compareKeyElements == NULL) return NULL;

	Map map = malloc(sizeof(*map));
	if (map == NULL) return NULL;

	MapNode head = malloc(sizeof(*head));// dummy head
	if (head == NULL) {
		free(map);
		return NULL;
	}
	head->keyElement = NULL;
	head->dataElement = NULL;
	head->next = NULL; // set to NULL (the list structure should be empty)

	map->size = 0;
	map->head = head;
	map->iterator = NULL;

	map->compare = compareKeyElements;
	map->copyMapData = copyDataElement;
	map->freeMapData = freeDataElement;
	map->copyMapKey = copyKeyElement;
	map->freeMapKey = freeKeyElement;

	return map;
}

//Gets a map object 
//Frees all the allocated memory
void mapDestroy(Map map) {
	if (map == NULL) {
		return;

	} else if (map->head == NULL){
        free(map);
        return;

    } else if (mapGetSize(map) > 0) {
            mapClear(map);
    } 
	free(map->head);
	free(map);
    //not freeing the map->iterator bc it's just a ptr(doesn't take any space)
}


Map mapCopy(Map map) {
	if (map == NULL) return NULL;

	Map copy = mapCreate(map->copyMapData, map->copyMapKey,
		map->freeMapData, map->freeMapKey,
		map->compare);
    //iterates through all the nodes
	MAPNODE_FOREACH(iter, map) {
		// if there's a failure
		if (mapPut(copy, iter->keyElement, iter->dataElement) != MAP_SUCCESS)
		{
			mapDestroy(copy);
			return NULL;
		}
		//else
		mapPut(copy, iter->keyElement, iter->dataElement);
	}

	//assigns iterator & size 
	copy->iterator = map->iterator;
	copy->size = mapGetSize(map);

	return copy;
}

int mapGetSize(Map map) {
	if (map == NULL) return -1;
	return map->size;
}

bool mapContains(Map map, MapKeyElement element) {
	MapNode result = mapNodeFindByKey(map, element);
	return (result != NULL) ? true : false;
}

MapResult
mapPut(Map map, MapKeyElement keyElement, MapDataElement dataElement) {
	if (map == NULL || keyElement == NULL ||
        dataElement == NULL) return MAP_NULL_ARGUMENT;

	if (mapContains(map, keyElement)) {
		MapNode result = mapNodeFindByKey(map, keyElement);
		MapDataElement tempData = mapGet(map, keyElement);
		result->dataElement = map->copyMapData(dataElement);
		map->freeMapData(tempData);
		map->iterator = NULL;
		return MAP_SUCCESS;
	} else {

		MapNode newNode = mapNodeCreate(map, keyElement, dataElement);
		if (newNode == NULL) return MAP_OUT_OF_MEMORY;

		MapNode nodeIterator = mapNodeGetFirst(map);
		if (nodeIterator == NULL)//in-case it's the first element
		{
			map->head->next = newNode;
			return mapUpdateInsertion(map);
		}

		while (nodeIterator->next != NULL) {
			//True when keyElement is bigger than iterator->key_element
			if (map->compare(nodeIterator->keyElement, keyElement) > 0) {
				MapNode prev = mapNodeGetPrev(map, nodeIterator);

				newNode->next = prev->next;
				prev->next = newNode;
				return mapUpdateInsertion(map);
			}
			nodeIterator = nodeIterator->next;
		}

		//checking the last node
		if (map->compare(nodeIterator->keyElement, keyElement) > 0) {
			MapNode prev = mapNodeGetPrev(map, nodeIterator);
			newNode->next = prev->next;
			prev->next = newNode;

			return mapUpdateInsertion(map);
		}
		//otherwise newNode should be last
		nodeIterator->next = newNode;
		return mapUpdateInsertion(map);
	}
}

MapDataElement mapGet(Map map, MapKeyElement keyElement) {
	if (map == NULL || keyElement == NULL) return NULL;

	if (mapContains(map, keyElement)) {
		MapNode result = mapNodeFindByKey(map, keyElement);
		return result->dataElement;
	}
	return NULL;
}

MapKeyElement mapGetFirst(Map map) {
	if (map == NULL || map->head == NULL ||
        map->head->next == NULL)return NULL;

	map->iterator = map->head->next;
	return map->iterator->keyElement;
}


MapResult mapRemove(Map map, MapKeyElement keyElement) {
	if (map == NULL || keyElement == NULL) return MAP_NULL_ARGUMENT;

	if (!mapContains(map, keyElement)) return MAP_ITEM_DOES_NOT_EXIST;

	//if it's the first element
	if (map->compare(mapGetFirst(map), keyElement) == 0) {
		MapNode newFirst = mapNodeGetFirst(map)->next;
		MapNode tmp = mapNodeGetFirst(map);
		map->head->next = newFirst;

		return mapUpdateRemoval(map, tmp);//handles a node removal
	}

	MapNode nodeIterator = mapNodeGetFirst(map);
	while (nodeIterator != NULL) {
		if (map->compare(nodeIterator->keyElement, keyElement) == 0) {
			MapNode prev = mapNodeGetPrev(map, nodeIterator);
			prev->next = nodeIterator->next;
			nodeIterator->next = NULL;
			return mapUpdateRemoval(map, nodeIterator);
		}
		nodeIterator = nodeIterator->next;
	}

	return -1;//shouldn't get here. I want to know if it did.
}

MapKeyElement mapGetNext(Map map) {
	if (map == NULL || map->iterator == NULL) return NULL;

	if (map->iterator->next == NULL) return NULL;

	map->iterator = map->iterator->next;
	return map->iterator->keyElement;
}


MapResult mapClear(Map map) {
	if (map == NULL || map->head == NULL) return MAP_NULL_ARGUMENT;

	while (map->head->next != NULL)
	{
		MapResult status = mapRemove(map, map->head->next->keyElement);
		if (status != MAP_SUCCESS)
			return status;
	}
	map->size = 0;
	return MAP_SUCCESS;
}

////////////////////    END Map part   //////////



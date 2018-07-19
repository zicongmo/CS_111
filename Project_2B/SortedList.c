#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sched.h>

/* Declare it for the extern */
int opt_yield = 0;

/* These functions all rely on the state of the list
   The critical section is the entire function */

void SortedList_insert(SortedList_t* list, SortedListElement_t* element){
	if(opt_yield & INSERT_YIELD){
		if(sched_yield() < 0){
			fprintf(stderr, "Error in sched_yield: %s\n", strerror(errno));
			exit(1);
		}
	}
	if(list->next == NULL){
		list->next = element;
		element->prev = list;
		element->next = NULL;
		return;
	}
	SortedListElement_t* current = list->next;
	while(current->next != NULL){
		if(strcmp(current->key, element->key) > 0){
			current->prev->next = element;
			element->prev = current->prev;
			current->prev = element;
			element->next = current;
			return;
		}
		current = current->next;
	}
	if(strcmp(current->key, element->key) > 0){
		current->prev->next = element;
		element->prev = current->prev;
		current->prev = element;
		element->next = current;
	}
	else{
		current->next = element;
		element->prev = current;
	}
}

int SortedList_delete(SortedListElement_t* element){
	if(opt_yield & DELETE_YIELD){
		if(sched_yield() < 0){
			fprintf(stderr, "Error in sched_yield: %s\n", strerror(errno));
			exit(1);
		}		
	}
	/* Last element in list */
	if(element->next == NULL){
		if(element->prev->next != element){
			return 1;
		}
		element->prev->next = NULL;
		return 0;
	}
	if(element->prev->next != element || element->next->prev != element){
		return 1;
	}
	element->prev->next = element->next;
	element->next->prev = element->prev;
	return 0;
}

SortedListElement_t* SortedList_lookup(SortedList_t* list, const char* key){
	if(opt_yield & LOOKUP_YIELD){
		if(sched_yield() < 0){
			fprintf(stderr, "Error in sched_yield: %s\n", strerror(errno));
			exit(1);
		}
	}
	SortedListElement_t* current = list->next;
	while(current != NULL){
		if(strcmp(current->key, key) == 0){
			return current;
		}
		current = current->next;
	}
	return NULL;
}

int SortedList_length(SortedList_t* list){
	if(opt_yield & LOOKUP_YIELD){
		if(sched_yield() < 0){
			fprintf(stderr, "Error in sched_yield: %s\n", strerror(errno));
			exit(1);
		}
	}
	SortedListElement_t* current = list->next;
	int count = 0;
	while(current != NULL){
		if(current->prev->next != current){
			return -1;
		}
		if(current->next != NULL){
			if(current->next->prev != current){
				return -1;
			}
		}
		count++;
		current = current->next;
	}

	return count;
}

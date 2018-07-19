#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "SortedList.h"

/* Values of options */
int num_threads = 1;
int num_iterations = 1;
int num_lists = 1;
char* yield;
int yield_i = 0;
int yield_d = 0;
int yield_l = 0;
int sync_s = 0;
int sync_m = 0;
const int KEY_LENGTH = 32;
const int ALPHA_LENGTH = 26;

SortedList_t** lists;
pthread_mutex_t** mutexes;
int** spin_locks;

unsigned int hash(const char* key){
	return (unsigned int)(key[0] * key[1]);
}

void sighandler(int signal){
	fprintf(stderr, "Error: Signal Received %d\n", signal);
	exit(1);
}

void* thread_func(void* args){
	SortedListElement_t* ptr = (SortedListElement_t* ) args;
	long long lock_time = 0;
	long long elapsed_time = 0;
	struct timespec start_time;
	struct timespec end_time;
	int c; 
	int i;
	for(i = 0; i < num_iterations; i++){
		unsigned int key = hash(ptr[i].key);
		pthread_mutex_t* lock = mutexes[key % num_lists];
		int* spin_lock = spin_locks[key % num_lists];

		/* Note time before getting lock */
		if(sync_m || sync_s){
			c = clock_gettime(CLOCK_MONOTONIC, &start_time);
			if(c < 0){
				fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
			}
		}

		if(sync_m){
			pthread_mutex_lock(lock);
			/* List should be assigned after lock is obtained */		
			SortedList_t* list = lists[key % num_lists];

			/* Note time after getting lock */
			if(sync_m || sync_s){
				c = clock_gettime(CLOCK_MONOTONIC, &end_time);
				if(c < 0){
					fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
					exit(1);
				}
				elapsed_time = (long long)((end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec));
				/* Add elapsed time to total time for the thread */
				lock_time += elapsed_time;
			}
			SortedList_insert(list, &(ptr[i]));
			pthread_mutex_unlock(lock);
		}
		else if(sync_s){
			while(__sync_lock_test_and_set(spin_lock, 1)){

			}
			/* List should be assigned after lock is obtained */		
			SortedList_t* list = lists[key % num_lists];

			/* Note time after getting lock */
			if(sync_m || sync_s){
				c = clock_gettime(CLOCK_MONOTONIC, &end_time);
				if(c < 0){
					fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
					exit(1);
				}
				elapsed_time = (long long)((end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec));
				/* Add elapsed time to total time for the thread */
				lock_time += elapsed_time;
			}
			SortedList_insert(list, &(ptr[i]));			
			__sync_lock_release(spin_lock);
		}
		else{
			SortedList_t* list = lists[key % num_lists];
			SortedList_insert(list, &(ptr[i]));
		}
	}

	/* If any of the sublists are corrupted, must exit(2) */
	for(i = 0; i < num_lists; i++){
		pthread_mutex_t* lock = mutexes[i];
		int* spin_lock = spin_locks[i];
		/* Note time before getting lock */
		if(sync_m || sync_s){
			c = clock_gettime(CLOCK_MONOTONIC, &start_time);
			if(c < 0){
				fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
			}
		}
		if(sync_m){
			pthread_mutex_lock(lock);
			/* List should be assigned after lock is obtained */
			SortedList_t* list = lists[i];
			/* Note time after getting lock */
			if(sync_m || sync_s){
				c = clock_gettime(CLOCK_MONOTONIC, &end_time);
				if(c < 0){
					fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
					exit(1);
				}
				/* Add elapsed time to total time for the thread */
				lock_time += (long long)(end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec);			
			}
			if(SortedList_length(list) == -1){
				fprintf(stderr, "Error: Corrupted pointers in list %d\n", i);
				exit(2);
			}
			pthread_mutex_unlock(lock);
		}
		else if(sync_s){
			while(__sync_lock_test_and_set(spin_lock, 1)){

			}
			/* List should be assigned after lock is obtained */
			SortedList_t* list = lists[i];
			/* Note time after getting lock */
			if(sync_m || sync_s){
				c = clock_gettime(CLOCK_MONOTONIC, &end_time);
				if(c < 0){
					fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
					exit(1);
				}
				/* Add elapsed time to total time for the thread */
				lock_time += (long long)(end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec);			
			}
			if(SortedList_length(list) == -1){
				fprintf(stderr, "Error: Corrupted pointers in list %d\n", i);
				exit(2);
			}
			__sync_lock_release(spin_lock);
		}
		else{
			/* List should be assigned after lock is obtained */
			SortedList_t* list = lists[i];
			if(SortedList_length(list) == -1){
				fprintf(stderr, "Error: Corrupted pointers in list %d\n", i);
				exit(2);
			}
		}
	}

	/* Lookup and delete */
	for(i = 0; i < num_iterations; i++){
		unsigned int key = hash(ptr[i].key);
		pthread_mutex_t* lock = mutexes[key % num_lists];
		int* spin_lock = spin_locks[key % num_lists];
		/* Note time before getting lock */
		if(sync_m || sync_s){
			c = clock_gettime(CLOCK_MONOTONIC, &start_time);
			if(c < 0){
				fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
			}
		}
		if(sync_m){
			pthread_mutex_lock(lock);
			/* List should be assigned after lock is obtained */		
			SortedList_t* list = lists[key % num_lists];

			/* Note time after getting lock */
			if(sync_m || sync_s){
				c = clock_gettime(CLOCK_MONOTONIC, &end_time);
				if(c < 0){
					fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
					exit(1);
				}
				/* Add elapsed time to total time for the thread */
				lock_time += (long long)(end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec);			
			}	
			SortedListElement_t* toDelete = SortedList_lookup(list, ptr[i].key);
			if(toDelete == NULL){
				fprintf(stderr, "Error: Element not found in list %d\n", key % num_lists);
				exit(2);
			}
			if(SortedList_delete(toDelete) != 0){
				fprintf(stderr, "Error: Corrupted pointer found while deleting from list %d\n", key % num_lists);
				exit(2);
			}
			pthread_mutex_unlock(lock);
		}
		else if(sync_s){
			while(__sync_lock_test_and_set(spin_lock, 1)){

			}
			/* List should be assigned after lock is obtained */		
			SortedList_t* list = lists[key % num_lists];
			/* Note time after getting lock */
			if(sync_m || sync_s){
				c = clock_gettime(CLOCK_MONOTONIC, &end_time);
				if(c < 0){
					fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
					exit(1);
				}
				/* Add elapsed time to total time for the thread */
				lock_time += (long long)(end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec);			
			}	
			SortedListElement_t* toDelete = SortedList_lookup(list, ptr[i].key);
			if(toDelete == NULL){
				fprintf(stderr, "Error: Element not found in list %d\n", key % num_lists);
				exit(2);
			}
			if(SortedList_delete(toDelete) != 0){
				fprintf(stderr, "Error: Corrupted pointer found while deleting from list %d\n", key % num_lists);
				exit(2);
			}
			__sync_lock_release(spin_lock);
		}
		else{
			/* List should be assigned after lock is obtained */		
			SortedList_t* list = lists[key % num_lists];
			SortedListElement_t* toDelete = SortedList_lookup(list, ptr[i].key);
			if(toDelete == NULL){
				fprintf(stderr, "Error: Element not found in list %d\n", key % num_lists);
				exit(2);
			}
			if(SortedList_delete(toDelete) != 0){
				fprintf(stderr, "Error: Corrupted pointer found while deleting from list %d\n", key % num_lists);
				exit(2);
			}
		}
	}
	return (void*) lock_time;
}

int main(int argc, char** argv){
	signal(SIGSEGV, sighandler);
	int a;
	while(1){
		static struct option long_options[] = {
			{"threads", required_argument, 0, 't'},
			{"iterations", required_argument, 0, 'i'},
			{"sync", required_argument, 0, 's'},
			{"yield", required_argument, 0, 'y'},
			{"lists", required_argument, 0, 'l'},
			{0, 0, 0, 0}
		};

		int option_index = 0;
		a = getopt_long(argc, argv, "", long_options, &option_index);
		if(a == -1){
			break;
		}
		int length;
		int i;
		switch(a){
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'i':
				num_iterations = atoi(optarg);
				break;
			/* We process it after the loop */
			case 'y':
				length = strlen(optarg);
				if(length > 3){
					fprintf(stderr, "Error: Argument to yield must be [ild]\n");
					exit(1);
				}
				for(i = 0; i < length; i++){
					switch(optarg[i]){
						case 'i':
							yield_i = 1;
							break;
						case 'd':
							yield_d = 1;
							break;
						case 'l':
							yield_l = 1;
							break;
						default:
							fprintf(stderr, "Error: Argument to yield must be [ild]\n");
							exit(1);
					}
				}
				break;
			/* We create the lists after */
			case 'l':
				num_lists = atoi(optarg);
				break;
			case 's':
				if(strcmp(optarg, "s") == 0){
					sync_s = 1;
					break;
				}
				else if(strcmp(optarg, "m") == 0){
					sync_m = 1;
					break;
				}
				else{
					fprintf(stderr, "Error: Argument to sync must be s, m\n");
					exit(1);
				}
			case '?':
				exit(1);
			default:
				break;
		}
	}

	opt_yield = 4*yield_l + 2*yield_d + yield_i;

	lists = malloc(sizeof(SortedList_t*) * num_lists);
	spin_locks = malloc(sizeof(int*) * num_lists);
	mutexes = malloc(sizeof(pthread_mutex_t*) * num_lists);
	int i;
	/* Initialize num_lists list heads, spin locks, and pthread_mutex_t */
	for(i = 0; i < num_lists; i++){
		SortedList_t* listHead = malloc(sizeof(SortedList_t));
		listHead->prev = NULL;
		listHead->next = NULL;
		listHead->key = NULL;
		lists[i] = listHead;

		int* sl = malloc(sizeof(int));
		*sl = 0;
		spin_locks[i] = sl;

		pthread_mutex_t* m = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(m, NULL);
		mutexes[i] = m;
	}

	SortedListElement_t* elements[num_threads];
	srand(time(NULL));
	int j;
	int k;
	for(i = 0; i < num_threads; i++){
		elements[i] = malloc(sizeof(SortedListElement_t) * num_iterations);
		for(j = 0; j < num_iterations; j++){
			char* randKey = malloc(sizeof(char) * KEY_LENGTH);
			/* Have to leave room for null character */
			for(k = 0; k < KEY_LENGTH - 1; k++){
				randKey[k] = (rand() % ALPHA_LENGTH) + 65;
			}
			randKey[KEY_LENGTH - 1] = 0;
			elements[i][j].key = randKey;
			elements[i][j].prev = NULL;
			elements[i][j].next = NULL;
		}
	}

	long long total_lock_time = 0;
	void* local_lock_time;
	/* Get start time */
	struct timespec start_time;
	int c = clock_gettime(CLOCK_MONOTONIC, &start_time);
	if(c < 0){
		fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
		exit(1);
	}

	/* Start the threads */
	pthread_t tid[num_threads];
	for(i = 0; i < num_threads; i++){
		pthread_create(&tid[i], NULL, thread_func, elements[i]);
	}

	for(i = 0; i < num_threads; i++){
		pthread_join(tid[i], &local_lock_time);
		total_lock_time += (long long) local_lock_time;
	}

	/* Get end time */
	struct timespec end_time;
	c = clock_gettime(CLOCK_MONOTONIC, &end_time);
	if(c < 0){
		fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
		exit(1);
	}

	/* No more list operations, don't have to unlock/lock */
	for(i = 0; i < num_lists; i++){
		SortedList_t* list = lists[i];
		if(SortedList_length(list) != 0){
			fprintf(stderr, "Error: List length nonzero\n");
			exit(2);
		}
	}

	int num_ops = num_iterations * num_threads * 3;
	long long runtime = (long long)(end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
	long long avg_runtime = runtime/ num_ops;

	int num_lock_ops = num_threads * (2 * num_iterations + num_lists);
	long long avg_locktime = total_lock_time/num_lock_ops;

	printf("list-");
	if(opt_yield & INSERT_YIELD){
		printf("i");
	}
	if(opt_yield & DELETE_YIELD){
		printf("d");
	}
	if(opt_yield & LOOKUP_YIELD){
		printf("l");
	}
	if(!opt_yield){
		printf("none");
	}
	printf("-");
	if(sync_s){
		printf("s");
	}
	else if(sync_m){
		printf("m");
	}
	else{
		printf("none");
	}
	printf(",%d,%d,%d,%d,%lld,%lld,%lld\n", num_threads, num_iterations, num_lists,
			num_ops, runtime, avg_runtime,avg_locktime);
	/* I'm too lazy to free all the key pointers */
	exit(0);
}

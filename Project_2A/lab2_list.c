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
char* yield;
int yield_i = 0;
int yield_d = 0;
int yield_l = 0;
int sync_s = 0;
int sync_m = 0;

SortedList_t* list;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int spin_lock = 0; /* 0 if free, 1 if occupied */

void sighandler(int signal){
	fprintf(stderr, "Error: Signal Received %d\n", signal);
	exit(1);
}

void* thread_func(void* args){
	SortedListElement_t* ptr = (SortedListElement_t* ) args;
	int i;
	for(i = 0; i < num_iterations; i++){
		if(sync_m){
			pthread_mutex_lock(&lock);
		}
		if(sync_s){
			while(__sync_lock_test_and_set(&spin_lock, 1)){

			}
		}
		SortedList_insert(list, &(ptr[i]));
		if(sync_m){
			pthread_mutex_unlock(&lock);
		}
		if(sync_s){
			__sync_lock_release(&spin_lock);
		}
	}
	if(sync_m){
		pthread_mutex_lock(&lock);
	}
	if(sync_s){
		while(__sync_lock_test_and_set(&spin_lock, 1)){

		}
	}
	if(SortedList_length(list) == -1){
		fprintf(stderr, "Error: Corrupted list\n");
		exit(2);
	}
	if(sync_m){
		pthread_mutex_unlock(&lock);
	}
	if(sync_s){
		__sync_lock_release(&spin_lock);
	}
	for(i = 0; i < num_iterations; i++){
		if(sync_m){
			pthread_mutex_lock(&lock);
		}
		if(sync_s){
			while(__sync_lock_test_and_set(&spin_lock, 1)){

			}
		}
		SortedListElement_t* toDelete = SortedList_lookup(list, ptr[i].key);
		if(toDelete == NULL){
			fprintf(stderr, "Error: Element not found\n");
			exit(2);
		}
		if(SortedList_delete(toDelete) == -1){
			fprintf(stderr, "Error: Corrupted pointer in delete\n");
			exit(2);
		}
		if(sync_m){
			pthread_mutex_unlock(&lock);
		}
		if(sync_s){
			__sync_lock_release(&spin_lock);
		}
	}
	return NULL;
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

	SortedList_t emptyList;
	emptyList.prev = NULL;
	emptyList.next = NULL;
	emptyList.key = NULL;
	list = &emptyList;
	SortedListElement_t* elements[num_threads];
	srand(time(NULL));
	int i;
	int j;
	for(i = 0; i < num_threads; i++){
		elements[i] = malloc(sizeof(SortedListElement_t) * num_iterations);
		for(j = 0; j < num_iterations; j++){
			char* randChar = malloc(sizeof(char));
			*randChar = rand() % 256;
			elements[i][j].key = randChar;
			elements[i][j].prev = NULL;
			elements[i][j].next = NULL;
		}
	}

	/* Get start time */
	struct timespec start_time;
	int c = clock_gettime(CLOCK_MONOTONIC, &start_time);
	if(c < 0){
		fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
	}

	/* Start the threads */
	pthread_t tid[num_threads];
	for(i = 0; i < num_threads; i++){
		pthread_create(&tid[i], NULL, thread_func, elements[i]);
	}

	for(i = 0; i < num_threads; i++){
		pthread_join(tid[i], NULL);
	}

	/* Get end time */
	struct timespec end_time;
	c = clock_gettime(CLOCK_MONOTONIC, &end_time);
	if(c < 0){
		fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
		exit(1);
	}

	if(SortedList_length(list) != 0){
		fprintf(stderr, "Error: List length nonzero\n");
		exit(2);
	}

	int num_ops = num_iterations * num_threads * 3;
	long long runtime = (long long)(end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
	long long avg_runtime = runtime/ num_ops;


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
	printf(",%d,%d,%d,%d,%lld,%lld\n", num_threads, num_iterations, 1,
			num_ops, runtime, avg_runtime);
	/* I'm too lazy to free all the key pointers */
	exit(0);
}

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

/* Values of arguments */
int num_threads = 1;
int num_iterations = 1;
int opt_yield = 0;
int sync_value = -1; /* 0 = c, 1 = m, 2 = s*/

/* Values of locks */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int spin_lock = 0; /* 0 if free, 1 if occupied */
int CAS = 0;

struct p_struct{
	long long* pointer;
	long long value;
};

void add(long long* pointer, long long value){
	long long sum = *pointer + value;
	if(opt_yield){
		if(sched_yield() < 0){
			fprintf(stderr, "Error in sched_yield: %s\n", strerror(errno));
			exit(1);
		}
	}
	*pointer = sum;
}

void add_m(long long* pointer, long long value){
	pthread_mutex_lock(&lock);
	add(pointer, value);
	pthread_mutex_unlock(&lock);
}

void add_s(long long* pointer, long long value){
	while(__sync_lock_test_and_set(&spin_lock, 1)){

	}
	add(pointer, value);
	__sync_lock_release(&spin_lock);
}

void add_c(long long* pointer, long long value){
	while(__sync_val_compare_and_swap(&CAS, 0, 1)){

	}
	add(pointer, value);
	__sync_val_compare_and_swap(&CAS, 1, 0);
}

void* thread_func(void* input){
	long long* p = (long long*) input;
	void (*correct_add)(long long* pointer, long long value);
	switch(sync_value){
		case 0:
			correct_add = add_c;
			break;
		case 1:
			correct_add = add_m;
			break;
		case 2:
			correct_add = add_s;
			break;
		default:
			correct_add = add;
			break;
	}
	int i;
	for(i = 0; i < num_iterations; i++){
		correct_add(p, 1);
	}
	for(i = 0; i < num_iterations; i++){
		correct_add(p, -1);
	}
	pthread_exit(NULL);
}

int main(int argc, char** argv){
	long long counter = 0;

	/* Get and process options */
	int a;
	while(1){
		static struct option long_options[] = {
			{"threads", required_argument, 0, 't'},
			{"iterations", required_argument, 0, 'i'},
			{"yield", no_argument, &opt_yield, 1},
			{"sync", required_argument, 0, 's'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		a = getopt_long(argc, argv, "", long_options, &option_index);
		if(a == -1){
			break;
		}
		switch(a){
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'i':
				num_iterations = atoi(optarg);
				break;
			case 's':
				if(strcmp(optarg, "c") == 0){
					sync_value = 0;
				}
				else if(strcmp(optarg, "m") == 0){
					sync_value = 1;
				}
				else if(strcmp(optarg, "s") == 0){
					sync_value = 2;
				}
				else{
					fprintf(stderr, "Error: Value of sync must be c, m, s\n");
					exit(1);
				}
				break;
			/* No work to be done */
			case 0:
				break;
			case '?':
				exit(1);
			default:
				break;
		}
	}

	/* Get start time */
	struct timespec start_time;
	int c = clock_gettime(CLOCK_MONOTONIC, &start_time);
	if(c < 0){
		fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
	}

	/* Create array of thread ids */
	pthread_t tid[num_threads];

	/* Initialize and run threads */
	int ret;
	int i;
	for(i = 0; i < num_threads; i++){
		ret = pthread_create(&tid[i], NULL, thread_func, (void*) &counter);
		/* This shouldn't fail but just in case */
		if(ret != 0){
			fprintf(stderr, "Error in pthread_create\n");
			exit(2);
		}
	}
	/* Join threads */
	for(i = 0; i < num_threads; i++){
		ret = pthread_join(tid[i], NULL);
		if(ret != 0){
			fprintf(stderr, "Error in pthread_join\n");
			exit(2);
		}
	}

	/* Get end time */
	struct timespec end_time;
	c = clock_gettime(CLOCK_MONOTONIC, &end_time);
	if(c < 0){
		fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
		exit(1);
	}

	char* test_name;
	if(opt_yield){
		if(sync_value == 0){
			test_name = "add-yield-c";
		}
		else if(sync_value == 1){
			test_name = "add-yield-m";
		}
		else if(sync_value == 2){
			test_name = "add-yield-s";
		}
		else{
			test_name = "add-yield-none";
		}
	}
	else{
		if(sync_value == 0){
			test_name = "add-c";
		}
		else if(sync_value == 1){
			test_name = "add-m";
		}
		else if(sync_value == 2){
			test_name = "add-s";
		}
		else{
			test_name = "add-none";
		}
	}

	int num_ops = num_threads * num_iterations * 2;
	long long runtime = (long long)(end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
	long long avg_runtime = runtime/ num_ops;

	printf("%s,%d,%d,%d,%lld,%lld,%lld\n", test_name, num_threads, num_iterations,
			num_ops, runtime, avg_runtime, counter);
	exit(0);
}

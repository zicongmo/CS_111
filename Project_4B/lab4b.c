#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h> 
#include <time.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <math.h>

/* Options */
int temp_scale = 'F';
int period = 1;
int log_fd = -1;
int stop = 0; 

/* Constants */
const int BLOCK_SIZE = 1024;
const int B = 4275;
const int R0 = 100000;
const int BIL = 1000000000;

/* Converts temperature from its raw value to C or F */
float convert_temperature(int sensor_temp){
	float R = 1023.0/sensor_temp - 1.0;
	R = R0*R;
	float temperature = 1.0/(log(R/R0)/B + 1/298.15) - 273.15;
	if(temp_scale == 'C'){
		return temperature;
	}
	return temperature*9/5 + 32;
}

/* Gets local time */
void get_time(struct tm** output){
	time_t raw_time;
	time(&raw_time);
	*output = localtime(&raw_time);
}

/* Places the report into string */
void generate_report(char* string, int* report_length, 
					 struct tm* time, float temperature, int shutdown){
	*report_length = sprintf(string, "%02d:%02d:%02d %.1f\n", 
							 time->tm_hour, time->tm_min, 
							 time->tm_sec, temperature);
	if(*report_length < 0){
		fprintf(stderr, "Error creating report\n");
		exit(1);
	}
	if(shutdown){
		*report_length = sprintf(string, "%02d:%02d:%02d SHUTDOWN\n", 
							 time->tm_hour, time->tm_min, 
							 time->tm_sec);	
		if(*report_length < 0){
			fprintf(stderr, "Error creating report\n");
			exit(1);
		}		
	}
}

/* The different types of commands */
void shutdown_handler(){
	/* Get time */
	struct tm time;
	struct tm* time_ptr = &time;
	get_time(&time_ptr);

	char* report = malloc(sizeof(char) * BLOCK_SIZE);
	/* Generate the report */
	int report_length;
	generate_report(report, &report_length, time_ptr, 0.0, 1);
	if(log_fd != -1){
		int w = write(log_fd, report, report_length);
		if(w < 0){
			fprintf(stderr, "Error in write: %s\n", strerror(errno));
			exit(1);
		}
	}
	/* This should always report regardless of the state of stop */
	printf("%s", report);
	free(report);
	exit(0);
}

void scale_handler(char* command){
	/* Technically ignores the case where input is like SCALE=F1 */
	char new_scale = command[6];
	if(new_scale != 'C' && new_scale != 'F'){
		fprintf(stderr, "Error: Argument to scale must be C or F\n");
	}
	else{
		temp_scale = new_scale;
	}
}

void period_handler(char* command){
	/* Not going to bother check if the thing after is invalid */
	int new_period = atoi(command + 7);
	if(new_period > 0){
		period = new_period;
	}
	else{
		fprintf(stderr, "Error: Argument to period must be positive\n");
	}
}

/* Recognizes and calls the correct handler for the command */
void execute_command(char* command){
	if(strncmp(command, "SCALE=", 6) == 0){
		scale_handler(command);
	}
	else if(strncmp(command, "PERIOD=", 7) == 0){
		period_handler(command);
	}
	else if(strcmp(command, "OFF") == 0){
		int length = strlen(command);
		command[length] = '\n';
		write(log_fd, command, length + 1);		
		shutdown_handler();
	}
	else if(strcmp(command, "STOP") == 0){
		stop = 1;
	}
	else if(strcmp(command, "START") == 0){
		stop = 0;
	}
	else if(strncmp(command, "LOG ", 4) == 0){

	}
	/* Don't write invalid commands to log files */
	else{
		fprintf(stderr, "Unrecognized command: %s\n", command);
		return;
	}
	int length = strlen(command);
	command[length] = '\n';
	write(log_fd, command, length + 1);
}

int main(int argc, char** argv){
	int a;
	while(1){
		static struct option long_options[] = {
			{"period", required_argument, 0, 'p'},
			{"scale", required_argument, 0, 's'},
			{"log", required_argument, 0, 'l'},
			{0, 0, 0 ,0}
		};
		int option_index = 0;
		a = getopt_long(argc, argv, "", long_options, &option_index);
		if(a == -1){
			break;
		}
		switch(a){
			case 'p':
				period = atoi(optarg);
				if(period <= 0){
					fprintf(stderr, "Error: Argument to period must be positive\n");
					exit(1);
				}
				break;
			case 's':
				if(strcmp(optarg, "C") == 0){
					temp_scale = 'C';
				}
				else if(strcmp(optarg, "F") == 0){
					temp_scale = 'F';
				}
				else{
					fprintf(stderr, "Error: Argument to scale must be C or F\n");
					exit(1);
				}
				break;
			case 'l':
				log_fd = open(optarg, O_CREAT | O_RDWR | O_NONBLOCK | O_TRUNC, 0666);
				if(log_fd < 0){
					fprintf(stderr, "Error in opening log: %s\n", strerror(errno));
					exit(1);
				}
				break;
			case '?':
				exit(1);
			default:
				break;
		}
	}

	/* Temperature sensor at A0 */
	mraa_aio_context temperatureSensor;
	temperatureSensor = mraa_aio_init(1);

	/* Button at GPIO_50, corresponding to MRAA 60 */
	mraa_gpio_context button;
	button = mraa_gpio_init(60);
	mraa_gpio_dir(button, MRAA_GPIO_IN);

	/* Initialize polling */
	struct pollfd pfd[1];
	pfd[0].fd = 0; /* stdin */
	pfd[0].events = POLLIN | POLLHUP | POLLERR;

	char* report = malloc(sizeof(char) * BLOCK_SIZE);
	char* input_command = malloc(sizeof(char) * BLOCK_SIZE);
	char* temp_buf = malloc(sizeof(char) * BLOCK_SIZE);

	/* Start and end indices of the current command */
	int start_index = 0; 
	int end_index = 0;

	while(1){
		/* Get temperature */
		int sensor_temp = mraa_aio_read(temperatureSensor);
		float temp = convert_temperature(sensor_temp);

		/* Get time */
		struct tm time;
		struct tm* time_ptr = &time;
		get_time(&time_ptr);

		/* Generate the report */
		int report_length;
		generate_report(report, &report_length, time_ptr, temp, 0);
		if(log_fd != -1 && !stop){
			int w = write(log_fd, report, report_length);
			if(w < 0){
				fprintf(stderr, "Error in write: %s\n", strerror(errno));
				exit(1);
			}
		}
		if(!stop){
			printf("%s", report);
		}

		/* Wait period seconds */
		struct timespec start_time;
		int c = clock_gettime(CLOCK_MONOTONIC, &start_time);
		if(c < 0){
			fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
			exit(1);			
		}
		struct timespec end_time;
		c = clock_gettime(CLOCK_MONOTONIC, &end_time);
		if(c < 0){
			fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
			exit(1);			
		}
		long long elapsed_nsec = (long long)(end_time.tv_sec - start_time.tv_sec)*BIL + (end_time.tv_nsec - start_time.tv_nsec); 
		/* This can still overflow but not gonna worry about that */
		while(elapsed_nsec < ((long long) period)*BIL){
			/* See if any input from stdin */
			int p = poll(pfd, 1, 0);
			if(p < 0){
				fprintf(stderr, "Error in poll: %s\n", strerror(errno));
				exit(1);
			}
			/* Pending input from stdin */
			if(pfd[0].revents & POLLIN){
				int r = read(pfd[0].fd, input_command + end_index, BLOCK_SIZE);
				if(r < 0){
					fprintf(stderr, "Error in read: %s\n", strerror(errno));
					exit(1);
				}
				int e = end_index;
				while(end_index < e + r){
					char c = input_command[end_index];
					if(c == '\n'){
						input_command[end_index] = '\0';
						execute_command(input_command + start_index);
						start_index = end_index + 1;
					}
					end_index++;
				}
				/* Save buffered input in case command wasn't finished */
				memcpy(temp_buf, input_command + start_index, end_index - start_index);
				memcpy(input_command, temp_buf, end_index - start_index);
				end_index -= start_index;
				start_index = 0;
			}

			if(mraa_gpio_read(button)){
				shutdown_handler();
			}

			c = clock_gettime(CLOCK_MONOTONIC, &end_time);
			if(c < 0){
				fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
				exit(1);			
			}
			elapsed_nsec = (long long)(end_time.tv_sec - start_time.tv_sec)*BIL + (end_time.tv_nsec - start_time.tv_nsec); 			
		}
	}
	free(report);
	free(input_command);
	exit(0);
}
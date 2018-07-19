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
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

/* Options */
int temp_scale = 'F';
int period = 1;
int log_fd = -1;
int stop = 0; 
int id = -1;
int port = -1;
char* host = "";
int host_specified = -1;

/* Variables */
int sock_fd;
SSL_CTX* context;
SSL* ssl;

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
		exit(2);
	}
	if(shutdown){
		*report_length = sprintf(string, "%02d:%02d:%02d SHUTDOWN\n", 
							 time->tm_hour, time->tm_min, 
							 time->tm_sec);	
		if(*report_length < 0){
			fprintf(stderr, "Error creating report\n");
			exit(2);
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
			exit(2);
		}
	}
	/* Send to server */
	SSL_write(ssl, report, report_length);
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


static struct option long_options[] = {
	{"period", required_argument, 0, 'p'},
	{"scale", required_argument, 0, 's'},
	{"id", required_argument, 0, 'i'},
	{"host", required_argument, 0, 'h'},
	{"log", required_argument, 0, 'l'},
	{0, 0, 0 ,0}
};

int main(int argc, char** argv){
	int a;
	int option_index = 0;
	while(1){
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
			case 'i':
				if(strlen(optarg) != 9){
					fprintf(stderr, "Error: ID argument must be 9 characters\n");
					exit(1);
				}
				else{
					id = atoi(optarg);
				}
				break;
			case 'h':
				host = optarg;
				host_specified = 1;
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

	if(id == -1){
		fprintf(stderr, "Error: No id parameter\n");
		exit(1);
	}

	if(host_specified == -1){
		fprintf(stderr, "Error: No hostname parameter\n");
		exit(1);
	}

	if(log_fd == -1){
		fprintf(stderr, "Error: No log parameter\n");
		exit(1);
	}

	/* Scan the arguments for the port argument */
	int num_ports = 0;
	for(a = 1; a < argc; a++){
		if(strncmp(argv[a], "-", 1) != 0){
			num_ports++;
			port = atoi(argv[a]);
		}
	}
	if(num_ports != 1){
		fprintf(stderr, "Error: Extra or missing port argument\n");
		exit(1);
	}


	/* Temperature sensor at A0 */
	mraa_aio_context temperatureSensor;
	temperatureSensor = mraa_aio_init(1);

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		fprintf(stderr, "Error in socket: %s\n", strerror(errno));
		exit(2);
	}

	struct hostent* server = gethostbyname(host);
	if(!server){
		fprintf(stderr, "Error: Could not find host %s\n", host);
		exit(2);
	}

	struct sockaddr_in serv;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);

	memcpy((char*) &serv.sin_addr.s_addr, (char*)server->h_addr, server->h_length);

	int c = connect(sock_fd, (struct sockaddr*) &serv, sizeof(serv));
	if(c < 0){
		fprintf(stderr, "Error in connect: %s\n", strerror(errno));
		exit(2);
	}

	int init = SSL_library_init();
	if(init < 0){
		fprintf(stderr, "Error initializing library\n");
		exit(2);
	}

	OpenSSL_add_all_algorithms();
	context = SSL_CTX_new(TLSv1_client_method());
	if(!context){
		fprintf(stderr, "Error creating context\n");
		exit(2);
	}

	ssl = SSL_new(context);
	if(SSL_set_fd(ssl, sock_fd) == 0){
		fprintf(stderr, "Error setting SSL's file descriptor\n");
		exit(2);
	}

	if(SSL_connect(ssl) != 1){
		fprintf(stderr, "Error establishing SSL connection\n");
		exit(2);
	}

	char* report = malloc(sizeof(char) * BLOCK_SIZE);

	/* Connection established, send ID number */
	int l = sprintf(report, "ID=%d\n", id);
	SSL_write(ssl, report, l);
	if(log_fd != -1){
		dprintf(log_fd, "ID=%d\n", id);
	}

	/* Initialize poll struct */
	struct pollfd pfd[1];
	pfd[0].fd = sock_fd; /* socket file descriptor */
	pfd[0].events = POLLIN | POLLHUP | POLLERR;

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
				exit(2);
			}
		}
		if(!stop){
			SSL_write(ssl, report, report_length);
		}

		/* Wait period seconds */
		struct timespec start_time;
		int c = clock_gettime(CLOCK_MONOTONIC, &start_time);
		if(c < 0){
			fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
			exit(2);			
		}
		struct timespec end_time;
		c = clock_gettime(CLOCK_MONOTONIC, &end_time);
		if(c < 0){
			fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
			exit(2);			
		}
		long long elapsed_nsec = (long long)(end_time.tv_sec - start_time.tv_sec)*BIL + (end_time.tv_nsec - start_time.tv_nsec); 
		/* This can still overflow but not gonna worry about that */
		while(elapsed_nsec < ((long long) period)*BIL){
			/* See if any input from stdin */
			int p = poll(pfd, 1, 0);
			if(p < 0){
				fprintf(stderr, "Error in poll: %s\n", strerror(errno));
				exit(2);
			}
			/* Pending input from server */
			if(pfd[0].revents & POLLIN){
				int r = SSL_read(ssl, input_command + end_index, BLOCK_SIZE);
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

			c = clock_gettime(CLOCK_MONOTONIC, &end_time);
			if(c < 0){
				fprintf(stderr, "Error in clock_gettime: %s\n", strerror(errno));
				exit(2);			
			}
			elapsed_nsec = (long long)(end_time.tv_sec - start_time.tv_sec)*BIL + (end_time.tv_nsec - start_time.tv_nsec); 			
		}
	}
	free(report);
	free(input_command);
	exit(0);
}
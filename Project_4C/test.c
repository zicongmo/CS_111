#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
/* Options */
int temp_scale = 'F';
int period = 1;
int log_fd = -1;
int stop = 0; 
int id = -1;
int port = -1;
char* host = "";
int host_specified = -1;

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

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		fprintf(stderr, "Error in socket: %s\n", strerror(errno));
		exit(2);
	}

	struct hostent* server = gethostbyname(host);
	if(!server){
		fprintf(stderr, "Error: Could not find host %s\n", host);
		exit(2);
	}

	printf("Got host by name\n");

	struct sockaddr_in serv;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);

	memcpy((char*) &serv.sin_addr.s_addr, (char*)server->h_addr, server->h_length);

	printf("Attempting to connect to server\n");

	int c = connect(sock_fd, (struct sockaddr*) &serv, sizeof(serv));
	if(c < 0){
		fprintf(stderr, "Error in connect: %s\n", strerror(errno));
		exit(2);
	}

	printf("Connected to server\n");

	/* Connection established, send ID number */
	dprintf(sock_fd, "ID=%d\n", id);
	if(log_fd != -1){
		dprintf(log_fd, "ID=%d\n", id);
	}

	printf("Wrote to server, exiting\n");

	close(sock_fd);
	exit(0);
}	

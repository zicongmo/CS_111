#include <unistd.h> /* read, write, close */
#include <fcntl.h> /* open */
#include <stdlib.h> /* malloc, realloc */
#include <errno.h> /* errno */
#include <string.h> /* strerror */
#include <getopt.h> /* getopt_long */
#include <stdio.h> /* fprintf */
#include <signal.h> /* signal */

void forceSegfault(){
	char* p = NULL;
	*p = 10;
}

void sighandler(int sig){
	fprintf(stderr, "Signal %d received, exiting with code 4\n", sig);
	exit(4);
}


static int segfault;
static int catch;

int main(int argc, char** argv){

	/* Option parsing */
	char* input_file = NULL;
	char* output_file = NULL;

	int a;

	while(1){

		static struct option long_options[] = {
			{"input", required_argument, NULL, 'i'}, 
			{"output", required_argument, NULL, 'o'}, 
			{"segfault", no_argument, &segfault, 1}, 
			{"catch", no_argument, &catch, 1},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		a = getopt_long(argc, argv, "", long_options, &option_index);

		/* No more options */
		if(a == -1){
			break;
		}

		switch(a){
			case 'i':
				input_file = optarg;
				break;
			case 'o':
				output_file = optarg;
				break;
			/* Flag cases */
			case 0:
				break;
			/* Unknown argument */
			case '?':
				exit(1);
		}

	}

	/* Implement options */

	/* If catch is specified, must create signal handler first to catch SIGSEGV */
	if(catch){
		signal(SIGSEGV, sighandler);
	}

	/* Immediately force segfault if option specified,
		not bothering to change fd around */
	if(segfault){
		forceSegfault();
	}

	/* If input was specified, redirect input */
	if(input_file){
		int fd = open(input_file, O_RDONLY);
		if(fd >= 0){
			close(0);
			dup(fd);
			close(fd);
		}
		/* Error! */
		else{
		  fprintf(stderr, "Error with argument --input=%s: %s\n", input_file, strerror(errno));
			exit(2);
		}
	}

	/* If output was specified, redirect output */
	if(output_file){
		int fd = creat(output_file, 0666);
		if(fd >= 0){
			close(1);
			dup(fd);
			close(fd);
		}
		/* Error! */
		else{
		  fprintf(stderr, "Error with argument --output=%s: %s\n", output_file, strerror(errno));
			exit(3);
		}
	}

	const int BLOCK_SIZE = 128;

	int len = 0;
	int buf_size = BLOCK_SIZE;
	void* buf = malloc(buf_size);
       
	int c = read(0, buf, BLOCK_SIZE);

	// Read BLOCK_SIZE bytes at a time until error/EOF
	while(c == BLOCK_SIZE){
		len += c;
		buf_size += BLOCK_SIZE;
		buf = realloc(buf, buf_size);
		c = read(0, buf+len, BLOCK_SIZE);
	}

	// Check for an error
	if(c == -1){
	  fprintf(stderr, "Error reading from input: %s", strerror(errno));
	}

	len += c;
	c = write(1, buf, len);
	if(c == -1){
	  fprintf(stderr, "Error writing to output: %s", strerror(errno));
	}

	free(buf);

	exit(0);
}

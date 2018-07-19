#include <termios.h> 
#include <unistd.h> /* read, write, exec */
#include <stdio.h> /* fprintf */
#include <string.h> /* strerror */
#include <errno.h> /* errno */
#include <stdlib.h> /* exit */
#include <signal.h> /* SIGINT */
#include <sys/types.h> /* kill */
#include <poll.h> /* poll */
#include <getopt.h> /* getopt_long */
#include <signal.h> /* signal */
#include <sys/types.h>
#include <sys/wait.h>

static int shell;
static int SIGPIPE_RECEIVED;

void restoreModes(struct termios* old){
	int t = tcsetattr(0, TCSANOW, old);
	if(t == -1){
		fprintf(stderr, "Error in tcsetattr: %s\n", strerror(errno));
		exit(1);
	}
}

/* Just because we want to restore terminal modes on receiving SIGPIPE */
void sighandler(int signal){
	if(signal == SIGPIPE){
		SIGPIPE_RECEIVED = 1;
	}
}

void exitHandler(int cpid, struct termios *old){
	if(shell){
		int status;
		int retval = waitpid(cpid, &status, 0);
		if(retval == -1){
			fprintf(stderr, "Error: %s\n", strerror(errno));
			restoreModes(old);
			exit(1);
		}
		fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", 
			WTERMSIG(status), WEXITSTATUS(status));
	}
	restoreModes(old);
	exit(0);
}

int main(int argc, char** argv){
	int a;
	signal(SIGPIPE, sighandler);
	while(1){
		static struct option long_options[] = {
			{"shell", no_argument, &shell, 1},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		a = getopt_long(argc, argv, "", long_options, &option_index);

		/* No more options */
		if(a == -1){
			break;
		}

		switch(a){
			case '0':
				break;
			case '?':
				exit(1);
		}
	}

	struct termios old, new;
	int t = tcgetattr(0, &old);	
	if(t == -1){
		fprintf(stderr, "Error in tcgetattr: %s\n", strerror(errno));
		exit(1);
	}

	new = old;

	/* Change the terminal modes */
	new.c_iflag = ISTRIP;
	new.c_oflag = 0;
	new.c_lflag = 0;
	t = tcsetattr(0, TCSANOW, &new);
	if(t == -1){
		fprintf(stderr, "Error in tcsetattr: %s\n", strerror(errno));
		exit(1);
	}

	int fd_term2shell[2];
	int fd_shell2term[2];
	int cpid = 0;

	struct pollfd pfd[2];

	pfd[0].fd = 0;
	pfd[0].events = POLLIN | POLLERR | POLLHUP;

	/* Fork a process, change its pipe, then exec bash */
	if(shell){

		/* After the call to pipe, fd contains 2 new file descriptors to use 
		   fd[0] is stdin, fd[1] is stdout */
		int p = pipe(fd_shell2term);
		if(p == -1){
			fprintf(stderr, "Error in pipe: %s\n", strerror(errno));
			restoreModes(&old);
			exit(1);
		}
		p = pipe(fd_term2shell);
		if(p == -1){
			fprintf(stderr, "Error in pipe: %s\n", strerror(errno));
			restoreModes(&old);
			exit(1);
		}

		cpid = fork();
		if(cpid == -1){
			fprintf(stderr, "Error in fork: %s\n", strerror(errno));
			restoreModes(&old);
			exit(1);
		}

		/* shell output */
		pfd[1].fd = fd_shell2term[0];
		pfd[1].events = POLLIN | POLLERR | POLLHUP;

		/* Child Process (the shell) */
		if(cpid  == 0){
			/* Read input from terminal */
			close(0);
			dup(fd_term2shell[0]);

			close(1);
			close(2);
			dup(fd_shell2term[1]);
			dup(fd_shell2term[1]);

			close(fd_shell2term[0]);
			close(fd_term2shell[1]);

			execl("/bin/bash", "bash", NULL);
		}
		/* Parent Process */
		else{
			/* Write to terminal */
			close(fd_shell2term[1]);
			close(fd_term2shell[0]);
		}
	}

	const int BLOCK_SIZE = 256;
	/* EOF */
	const char EXIT_CODE = 4;
	void* buf = malloc(BLOCK_SIZE);
	/* The character(s) to write to standard output */
	char writeBuf[BLOCK_SIZE];
	/* The character to send to the shell */
	char shellChar;

	int bytesToWrite;
	int exitCodeDetected = 0;

	int retval;

	while(!exitCodeDetected){
		retval = poll(pfd, 2, 0);
		if(retval < 0){
			fprintf(stderr, "Error: %s\n", strerror(errno));
			restoreModes(&old);
			exit(1);
		}
		/* Pending input from keyboard */
		if(pfd[0].revents & POLLIN){
			t = read(0, buf, BLOCK_SIZE);
			if(t == -1){
				fprintf(stderr, "Error: %s\n", strerror(errno));
				restoreModes(&old);
				exit(1);
			}
			int i;
			bytesToWrite = 0;
			for(i = 0; i < t; i++){
				char c = ((char*) buf)[i];
				if(c == '\r' || c == '\n'){
					writeBuf[bytesToWrite++] = '\r';
					writeBuf[bytesToWrite++] = '\n';
					shellChar = '\n';
				}
				else if(c == EXIT_CODE){
					exitCodeDetected = 1;
					if(shell){
						close(fd_term2shell[1]);
					}
					break;
				}
				else if(c == 3 && shell){
					bytesToWrite = 0; 
					if(kill(cpid, SIGINT) == -1){
						fprintf(stderr, "Error: %s\n", strerror(errno));
						restoreModes(&old);
						exit(1);					
					}
				}
				else{
					writeBuf[bytesToWrite++] = c;
					shellChar = c;
				}
				if(shell){
					t = write(fd_term2shell[1], (void *)(&shellChar), 1);
					if(t == -1){
						fprintf(stderr, "Error: %s\n", strerror(errno));
						restoreModes(&old);
						exit(1);
					}	
				}
			}
			t = write(1, (void*) writeBuf, bytesToWrite);
			if(t == -1){
				fprintf(stderr, "Error: %s\n", strerror(errno));
				restoreModes(&old);
				exit(1);
			}
		}
		if(pfd[1].revents & POLLIN){
			t = read(fd_shell2term[0], buf, BLOCK_SIZE);
			if(t == -1){
				fprintf(stderr, "Error: %s\n", strerror(errno));
				restoreModes(&old);
				exit(1);
			}
			if(t == 0){
				exitHandler(cpid, &old);
			}
			bytesToWrite = 0;
			int i;
			for(i = 0; i < t; i++){
				char c = ((char*) buf)[i];
				if(c == '\r' || c == '\n'){
					writeBuf[bytesToWrite++] = '\r';
					writeBuf[bytesToWrite++] = '\n';
				}
				else if(c == EOF){
					exitHandler(cpid, &old);
				}
				else{
					writeBuf[bytesToWrite++] = c;
				}
			}
			t = write(1, (void*) writeBuf, bytesToWrite);
			if(t == -1){
				fprintf(stderr, "Error: %s\n", strerror(errno));
				restoreModes(&old);
				exit(1);
			}
		}
		if(pfd[1].revents & POLLHUP || pfd[1].revents & POLLERR){
			exitHandler(cpid, &old);
		}
		if(SIGPIPE_RECEIVED){
			exitHandler(cpid, &old);
		}
	}
	exitHandler(cpid, &old);
	/* Just in case */
	exit(0);
}

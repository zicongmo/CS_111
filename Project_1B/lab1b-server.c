#include <getopt.h> /* getopt_long */ 
#include <stdio.h> /* fprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write */
#include <sys/types.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h> /* socket */
#include <string.h> /* strerror */
#include <errno.h> /* errno */
#include <poll.h> /* poll */
#include <signal.h> 
#include <sys/wait.h>

#include "zlib.h"

int cpid = 0;
int sock_fd;

void exitHandler(){
	int status;
	int wp = waitpid(cpid, &status, 0);
	if(wp < 0){
		fprintf(stderr, "Error in waitpid: %s\n", strerror(errno));
		exit(1);
	}
	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", 
		WTERMSIG(status), WEXITSTATUS(status));
	if(close(sock_fd) < 0){
		fprintf(stderr, "Error closing socket fd: %s\n", strerror(errno));
		exit(1);
	}
	exit(0);
}

void sighandler(int signal){
	/* This is just here to surpress the warning */
	if(signal == SIGPIPE){
		exitHandler();
	}
}

void compressBuffer(void* input, int i_bytes, void* output, int* o_bytes){
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	strm.avail_in = i_bytes;
	strm.next_in = input;
	strm.avail_out = 256;
	strm.next_out = output;
	/* just assume it works */
	deflateInit(&strm, Z_DEFAULT_COMPRESSION);
	deflate(&strm, Z_FINISH);
	deflateEnd(&strm);

	*o_bytes = strm.total_out;
}

void inflateBuffer(void* input, int i_bytes, void* output, int* o_bytes){
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	strm.avail_in = i_bytes;
	strm.next_in = input;
	strm.avail_out = 256;
	strm.next_out = output;

	inflateInit(&strm);
	inflate(&strm, Z_NO_FLUSH);
	inflateEnd(&strm);

	*o_bytes = strm.total_out;
}

static int compressed = 0;

int main(int argc, char** argv){
	int port = -1;
	int a;
	while(1){
		static struct option long_options[] = {
			{"port", required_argument, 0, 'p'},
			{"compress", no_argument, &compressed, 1},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		a = getopt_long(argc, argv, "", long_options, &option_index);

		/* No more options */
		if(a == -1){
			break;
		}

		switch(a){
			case 'p':
				port = atoi(optarg);
				break;
			case '?':
				exit(1);
			default: 
				break;
		}
	}

	/* port option not specified */
	if(port == -1){
		fprintf(stderr, "Port argument not specified\n");
		exit(1);
	}

	/* ========== Establish socket ========== */
	socklen_t c_len;
	struct sockaddr_in serv, cli;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	memset(&cli, 0, sizeof(struct sockaddr_in));
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		fprintf(stderr, "Error in socket: %s\n", strerror(errno));
		exit(1);
	}

	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);

	int b = bind(sock_fd, (struct sockaddr*) &serv, sizeof(serv));
	if(b < 0){
		fprintf(stderr, "Error in bind: %s\n", strerror(errno));
		exit(1);
	}
	listen(sock_fd, 5);

	c_len = sizeof(cli);
	sock_fd = accept(sock_fd, (struct sockaddr*) &cli, &c_len);
	if(sock_fd < 0){
		fprintf(stderr, "Error in accept: %s\n", strerror(errno));
		exit(1);
	}

	/* ========== Set up child process ========== */
	/* Create two pipes. Read end of pipe is 0, write end is 1 */
	int s2t[2];
	int t2s[2];
	int p = pipe(s2t);
	if(p < 0){
		fprintf(stderr, "Error in pipe: %s\n", strerror(errno));
		exit(1);
	}
	p = pipe(t2s);
	if(p < 0){
		fprintf(stderr, "Error in pipe: %s\n", strerror(errno));
		exit(1);
	}

	/* Fork a new process */
	cpid = fork();
	if(cpid < 0){
		fprintf(stderr, "Error in fork: %s\n", strerror(errno));
		exit(1);
	}

	signal(SIGPIPE, sighandler);

	/* Set up polling */
	struct pollfd pfd[2];
	/* Monitor client input */
	pfd[0].fd = sock_fd;
	pfd[0].events = POLLIN | POLLERR | POLLHUP;
	/* Monitor shell output */
	pfd[1].fd = s2t[0];
	pfd[1].events = POLLIN | POLLERR | POLLHUP;

	/* Child */
	if(cpid == 0){
		/* Redirect file descriptors */
		close(0);
		dup(t2s[0]);
		close(1);
		close(2);
		dup(s2t[1]);
		dup(s2t[1]);

		close(t2s[0]);
		close(s2t[1]);
		/* Close write end of t2s, and read end of s2t */
		close(t2s[1]);
		close(s2t[0]);

		execl("/bin/bash", "bash", NULL);
	}
	/* Parent */
	else{
		/* Close write end of s2t, and read end of t2s */
		close(s2t[1]);
		close(t2s[0]);
	}


	/* ========== Read input from client ========== */
	const int BLOCK_SIZE = 256;
	void* buf = malloc(BLOCK_SIZE);
	char compressedBuf[BLOCK_SIZE]; /* Buffer to store compressed data */
	char writeBuf[BLOCK_SIZE]; /* Buffer for shell writes */

	/* This should be changed eventually */
	while(1){
		/* Poll between client and shell output */
		p = poll(pfd, 2, 0);
		if(p < 0){
			fprintf(stderr, "Error in poll: %s\n", strerror(errno));
			exit(1);
		}

		/* Pending input from client */
		if(pfd[0].revents & POLLIN){
			/* Read a block of input, and process all 
			   compressedBuf may not actually be compressed data. */
			int r = read(pfd[0].fd, compressedBuf, BLOCK_SIZE);
			if(r < 0){
				fprintf(stderr, "Error in client read: %s\n", strerror(errno));
				exit(1);
			}
			/* Is t2s writeable? */
			int isWritable = 1;
			/* Number of bytes to write to shell */
			int num_bytes = 0;

			/* Data is compressed, inflate before processing */
			if(compressed){
				int output_bytes = 0;
				inflateBuffer(compressedBuf, r, buf,&output_bytes);
				r = output_bytes;
			}
			/* If data is not compressed, copy contents over into buf */
			else{
				memcpy(buf, compressedBuf, r);
			}

			int i;
			for(i = 0; i < r; i++){
				char c = ((char*) buf)[i];
				switch(c){
					/* ^C  */
					case 3:
						num_bytes = 0;
						if(kill(cpid, SIGINT) < 0){
							fprintf(stderr, "Error killing child: %s\n", strerror(errno));
							exit(1);
						}
						break; 
					/* ^D */
					case 4:
						/* In case we get 2 ^D in one buffer */
						if(isWritable){
							if(close(t2s[1]) < 0){
								fprintf(stderr, "Error in close 1: %s\n", strerror(errno));
								exit(1);
							}
							isWritable = 0;
						}
						break;
					case '\r':
					case '\n':
						writeBuf[num_bytes++] = '\n';
						break;
					default:
						writeBuf[num_bytes++] = c;
						break;
				}
			}
			if(r == 0 && isWritable){
				if(close(t2s[1]) < 0){
					fprintf(stderr, "Error in close 2: %s\n", strerror(errno));
					exit(1);
				}
				isWritable = 0;
			}
			if(isWritable){
				int w = write(t2s[1], (void*) writeBuf, num_bytes);
				if(w < 0){
					fprintf(stderr, "Error in shell write: %s\n", strerror(errno));
					exit(1);
				}
			}
		}

		/* Pending input from shell */
		if(pfd[1].revents & POLLIN){
			/* Read a block of input, and process all */
			int r = read(pfd[1].fd, buf, BLOCK_SIZE);
			if(r < 0){
				fprintf(stderr, "Error in shell read: %s\n", strerror(errno));
				exit(1);
			}
			/* Number of bytes to send to client */
			int num_bytes = 0;
			int i;
			for(i = 0; i < r; i++){
				char c = ((char*) buf)[i];
				switch(c){
					case 4:
						exitHandler();
						break;
					default:
						writeBuf[num_bytes++] = c;
						break;
				}
			}
			/* EOF */
			if(r == 0){
				exitHandler();
			}
			else{
				if(compressed){
					int output_bytes = 0;
					compressBuffer(writeBuf, num_bytes, compressedBuf, &output_bytes);
					memcpy(writeBuf, compressedBuf, output_bytes);
					num_bytes = output_bytes;
				}

				int w = write(sock_fd, (void*) writeBuf, num_bytes);
				if(w < 0){
					fprintf(stderr, "Error in shell write: %s\n", strerror(errno));
					exit(1);
				}	
			}
		}

		/* Read error from client */
		if(pfd[0].revents & POLLHUP || pfd[0].revents & POLLERR){
			if(close(t2s[1]) < 0){
				fprintf(stderr, "Error in close 3: %s\n", strerror(errno));
				exit(1);
			}			
		}		
		/* Read error from pipe */
		if(pfd[1].revents & POLLHUP || pfd[1].revents & POLLERR){
			exitHandler();
		}
	}

	close(sock_fd);
	return 0;
}

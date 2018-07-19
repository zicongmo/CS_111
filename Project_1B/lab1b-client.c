#include <termios.h>
#include <getopt.h> /* getopt_long */ 
#include <stdio.h> /* fprintf, dprintf */
#include <stdlib.h> /* exit */
#include <sys/socket.h> /* socket */
#include <errno.h> /* errno */
#include <string.h> /* strerror */
#include <fcntl.h> /* open */
#include <sys/types.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>

#include "zlib.h"

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

void restoreModes(struct termios* old){
	int t = tcsetattr(0, TCSANOW, old);
	if(t < 0){
		fprintf(stderr, "Error in tcsetattr: %s\n", strerror(errno));
		exit(1);
	}
}

static int compression = 0;

int main(int argc, char** argv){
	/* ========== Parse options ========== */
	int port = -1;
	int log_fd;
	char* log;
	int a;
	while(1){
		static struct option long_options[] = {
			{"port", required_argument, 0, 'p'},
			{"log", required_argument, 0, 'l'},
			{"compress", no_argument, &compression, 1},
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
			case 'l':
				log = optarg;
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

	/* log specified */
	if(log){
		log_fd = open(log, O_CREAT | O_RDWR, 0666);
		if(log_fd < 0){
			fprintf(stderr, "Error in opening log: %s\n", strerror(errno));
			exit(1);
		}
	}


	/* ========== Place terminal in no-echo mode ========== */ 
	struct termios old, new;
	int t = tcgetattr(0, &old);	
	if(t < 0){
		fprintf(stderr, "Error in tcgetattr: %s\n", strerror(errno));
		exit(1);
	}
	/* Change the terminal modes */
	new = old;
	new.c_iflag = ISTRIP;
	new.c_oflag = 0;
	new.c_lflag = 0;
	t = tcsetattr(0, TCSANOW, &new);
	if(t < 0){
		fprintf(stderr, "Error in tcsetattr: %s\n", strerror(errno));
		exit(1);
	}



	/* ========== Establish socket ========== */
	int sock_fd;
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		fprintf(stderr, "Error in socket: %s\n", strerror(errno));
		restoreModes(&old);
		exit(1);
	}

	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);

	int c = connect(sock_fd, (struct sockaddr*) &serv, sizeof(serv));
	if(c < 0){
		fprintf(stderr, "Error in connect: %s\n", strerror(errno));
		restoreModes(&old);
		exit(1);
	}

	/* Set up polling */
	struct pollfd pfd[2];
	pfd[0].fd = 0;
	pfd[0].events = POLLIN | POLLERR | POLLHUP;
	pfd[1].fd = sock_fd;
	pfd[1].events = POLLIN | POLLERR | POLLHUP;


	/* ========== Send keyboard input to server ========== */
	const int BLOCK_SIZE = 256;
	void* buf = malloc(BLOCK_SIZE);
	char compressedBuf[BLOCK_SIZE];
	char writeBuf[BLOCK_SIZE];
	while(1){
		int p = poll(pfd, 2, 0);
		if(p < 0){
			fprintf(stderr, "Error in poll: %s\n", strerror(errno));
			restoreModes(&old);
			exit(1);
		}

		if(pfd[0].revents & POLLIN){
			/* Read keyboard input into buffer */
			int r = read(0, buf, BLOCK_SIZE);
			if(r < 0){
				fprintf(stderr, "Error in read: %s\n", strerror(errno));
				restoreModes(&old);
				exit(1);			
			}

			int num_bytes = 0;
			int i;
			for(i = 0; i < r; i++){
				char c = ((char*) buf)[i];
				switch(c){
					case '\r':
					case '\n':
						writeBuf[num_bytes++] = '\r';
						writeBuf[num_bytes++] = '\n';
						break;
					default:
						writeBuf[num_bytes++] = c;
						break;
				}
			}

			/* Compress the buffer before sending it to server */
			if(compression){
				int compressed_size = 0;
				compressBuffer(buf, r, (void*) compressedBuf, &compressed_size);
				memcpy(buf, compressedBuf, compressed_size);
				r = compressed_size;			
			}

			/* End in null character for easier log writing */
			((char*) buf)[r] = '\0';

			/* Write buffer to server */
			int w = write(sock_fd, buf, r);
			if(w < 0){
				fprintf(stderr, "Error in server write: %s\n", strerror(errno));
				restoreModes(&old);
				exit(1);			
			}
			/* Write buffer to stdout */
			w = write(1, (void*) writeBuf, num_bytes);
			if(w < 0){
				fprintf(stderr, "Error in server write: %s\n", strerror(errno));
				restoreModes(&old);
				exit(1);	
			}
			/* Write buffer to log */
			if(log){
				dprintf(log_fd, "SENT %d bytes: %s\n", r, (char*) buf);
			}
		}

		if(pfd[1].revents & POLLIN){
			int r = read(sock_fd, compressedBuf, BLOCK_SIZE);
			if(r < 0){
				fprintf(stderr, "Error in read: %s\n", strerror(errno));
				restoreModes(&old);
				exit(1);			
			}
			if(r == 0){
				break;
			}
			/* Log before decompression happens */
			((char*) compressedBuf)[r] = '\0';
			if(log){
				dprintf(log_fd, "RECEIVED %d bytes: %s\n", r, (char*) compressedBuf);
			}
			
			if(compression){
				int output_bytes = 0;
				inflateBuffer(compressedBuf, r, buf, &output_bytes);
				r = output_bytes;
			}
			else{
				memcpy(buf, compressedBuf, r);
			}

			int num_bytes = 0;
			int i;
			for(i = 0; i < r; i++){
				char c = ((char*) buf)[i];
				switch(c){
					case '\r':
					case '\n':
						writeBuf[num_bytes++] = '\r';
						writeBuf[num_bytes++] = '\n';
						break;
					default:
						writeBuf[num_bytes++] = c;
						break;
				}
			}
			/* Write buffer to stdout */
			int w = write(1, (void*) writeBuf, num_bytes);
			if(w < 0){
				fprintf(stderr, "Error in server write: %s\n", strerror(errno));
				restoreModes(&old);
				exit(1);	
			}
		}	
		if(pfd[1].revents & POLLHUP || pfd[1].revents & POLLERR){
			break;
		}
	}

	restoreModes(&old);
	close(sock_fd);
	return 0;
}

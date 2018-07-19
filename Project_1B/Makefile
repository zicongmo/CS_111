default: lab1b-client.c lab1b-server.c
	gcc -Wall -Wextra -lz -o lab1b-client lab1b-client.c
	gcc -Wall -Wextra -lz -o lab1b-server lab1b-server.c
client: lab1b-client.c
	gcc -Wall -Wextra -lz -o lab1b-client lab1b-client.c
server: lab1b-server.c
	gcc -Wall -Wextra -lz -o lab1b-server lab1b-server.c	
clean:
	rm -rf lab1b-server lab1b-client lab1b-804654167.tar.gz
dist:
	tar -zcvf lab1b-804654167.tar.gz Makefile README lab1b-server.c lab1b-client.c
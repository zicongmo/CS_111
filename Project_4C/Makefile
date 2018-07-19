default:
	gcc -Wall -Wextra -lm -lmraa -o lab4c_tcp lab4c_tcp.c
	gcc -Wall -Wextra -lm -lmraa -lssl -lcrypto -o lab4c_tls lab4c_tls.c
clean:
	rm -f lab4c_tls lab4c_tcp lab4c-804654167.tar.gz
dist:
	tar -zvcf lab4c-804654167.tar.gz lab4c_tls.c lab4c_tcp.c Makefile README

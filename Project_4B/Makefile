default:
	gcc -Wall -Wextra -std=gnu99 -lm -lmraa -o lab4b lab4b.c
check:
	-./lab4b --bad_argument
	echo "SCALE=F\nSTOP\nPERIOD=1\nSTART\nLOG ASDF\nOFF\n" | ./lab4b --period=2 --scale=C --log=log_check.txt 
	echo "BAD_COMMAND\nSTOP\nOFF\n" | ./lab4b --log=log_check.txt
	-grep "BAD_COMMAND" log_check.txt > /dev/null
	rm -f log_check.txt
clean:
	rm -f lab4b lab4b-804654167.tar.gz
dist:
	tar -zcvf lab4b-804654167.tar.gz lab4b.c Makefile README
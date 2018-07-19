default:
	gcc -Wall -Wextra -lprofiler -g -lpthread -o lab2_list SortedList.c lab2_list.c
tests: default
	./lab2_list --iterations=1000 --threads=1 --sync=m > lab2b_list.csv
	./lab2_list --iterations=1000 --threads=2 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=8 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=12 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=16 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=24 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=1 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=2 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=8 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=12 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=16 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=24 --sync=s >> lab2b_list.csv
	-./lab2_list --iterations=1 --threads=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=2 --threads=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=4 --threads=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=8 --threads=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=16 --threads=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=1 --threads=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=2 --threads=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=4 --threads=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=8 --threads=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=16 --threads=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=1 --threads=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=2 --threads=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=4 --threads=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=8 --threads=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=16 --threads=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=1 --threads=12 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=2 --threads=12 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=4 --threads=12 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=8 --threads=12 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=16 --threads=12 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=1 --threads=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=2 --threads=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=4 --threads=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=8 --threads=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --iterations=16 --threads=16 --yield=id --lists=4 >> lab2b_list.csv	
	./lab2_list --iterations=10 --threads=1 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=1 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=1 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=1 --yield=id --lists=4 --sync=m >> lab2b_list.csv	 
	./lab2_list --iterations=10 --threads=4 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=4 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=4 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=4 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=10 --threads=8 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=8 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=8 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=8 --yield=id --lists=4 --sync=m >> lab2b_list.csv	
	./lab2_list --iterations=10 --threads=12 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=12 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=12 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=12 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=10 --threads=16 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=16 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=16 --yield=id --lists=4 --sync=m >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=16 --yield=id --lists=4 --sync=m >> lab2b_list.csv			
	./lab2_list --iterations=10 --threads=1 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=1 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=1 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=1 --yield=id --lists=4 --sync=s >> lab2b_list.csv	 
	./lab2_list --iterations=10 --threads=4 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=4 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=4 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=4 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=10 --threads=8 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=8 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=8 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=8 --yield=id --lists=4 --sync=s >> lab2b_list.csv	
	./lab2_list --iterations=10 --threads=12 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=12 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=12 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=12 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=10 --threads=16 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=20 --threads=16 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=40 --threads=16 --yield=id --lists=4 --sync=s >> lab2b_list.csv 
	./lab2_list --iterations=80 --threads=16 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=1 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=2 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=4 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=8 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=12 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=1 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=2 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=4 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=8 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=12 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=1 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=2 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=4 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=8 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=12 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=1 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=2 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=4 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=8 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=12 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=1 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=2 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=4 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=8 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=12 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=1 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=2 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=4 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=8 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --iterations=1000 --threads=12 --lists=16 --sync=s >> lab2b_list.csv
graphs: tests
	./lab2b_list.gp
profile: default
	LDPRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=./raw.out ./lab2_list --iterations=1000 --threads=12 --sync=s
	pprof ./lab2_list ./raw.out --text > profile.out
	pprof --list=thread_func ./lab2_list ./raw.out >> profile.out
	rm -f raw.out
dist: profile graphs
	tar -zcvf lab2b-804654167.tar.gz lab2_list.c Makefile SortedList.c SortedList.h README lab2b_list.csv lab2b_list.gp *.png profile.out 
clean:
	rm -f lab2b-804654167.tar.gz lab2_list
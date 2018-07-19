#! /usr/bin/gnuplot

set terminal png
set datafile separator ","

set title "Lab2b-1: Number of operations per second vs Threads"
set xlabel "Threads"
set ylabel "Number of operations per second"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'mutex' with points lc rgb 'red', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'spin-lock' with points lc rgb 'green'

set title "Lab2b-2: Wait-for-lock time and Average Time per operation"
set xlabel "Threads"
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'Time per operation' with points lc rgb 'red', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'Wait-for-lock time' with points lc rgb 'green'

set title "Lab2b-3: Successful Tests"
set xlabel "Threads"
set ylabel "Iterations"
set output 'lab2b_3.png'

plot \
     "< grep 'list-id-none,' lab2b_list.csv" using ($2):($3) \
	title 'No synchronization' with points lc rgb 'red', \
     "< grep 'list-id-s,' lab2b_list.csv" using ($2):($3) \
	title 'Spinlock synchronization' with points lc rgb 'blue', \
     "< grep 'list-id-m,' lab2b_list.csv" using ($2):($3) \
	title 'Mutex synchronization' with points lc rgb 'green'

set title "Lab2b-4: Number of operations per second vs Threads (Mutexes)"
set xlabel "Threads"
set ylabel "Number of operations per second"
set logscale y 10
set output 'lab2b_4.png'

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '1 list' with points lc rgb 'red', \
     "< grep -e 'list-none-m,[0-9]*,1000,4' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '4 lists' with points lc rgb 'green', \
     "< grep -e 'list-none-m,[0-9]*,1000,8' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '8 lists' with points lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,16' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '16 lists' with points lc rgb 'orange'

set title "Lab2b-5: Number of operations per second vs Threads (Spin Locks)"
set xlabel "Threads"
set ylabel "Number of operations per second"
set logscale y 10
set output 'lab2b_5.png'

plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '1 list' with points lc rgb 'red', \
     "< grep -e 'list-none-s,[0-9]*,1000,4' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '4 lists' with points lc rgb 'green', \
     "< grep -e 'list-none-s,[0-9]*,1000,8' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '8 lists' with points lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,16' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title '16 lists' with points lc rgb 'orange'

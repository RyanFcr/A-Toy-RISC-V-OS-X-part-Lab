all : rand.o string.o printk.o

printk.o:
	${GCC} ${CFLAG} -c printk.c

rand.o :
	${GCC} ${CFLAG} -c rand.c

string.o:
	${GCC} ${CFLAG} -c string.c


clean: 
	rm rand.o string.o printk.o

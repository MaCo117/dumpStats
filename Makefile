CFLAGS=-std=c++11
PROJ=dumpStats
CC=g++
RM=rm -f
LDFLAGS = -lm
SRC=src/


${PROJ} : dumpStats.o objects.o
	${CC} ${CFLAGS} dumpStats.o objects.o ${LDFLAGS} -o ${PROJ}

objects.o : ${SRC}objects.cpp ${SRC}objects.H
	${CC} ${CFLAGS} -c ${SRC}objects.cpp
	
dumpStats.o : ${SRC}dumpStats.cpp ${SRC}objects.H
	${CC} ${CFLAGS} -c ${SRC}dumpStats.cpp


clean:
	$(RM) *.o
	$(RM) $(PROJ)

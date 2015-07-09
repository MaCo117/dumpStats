CFLAGS=-std=c++11
PROJ=dumpStats
CC=g++
RM=rm -f
LDFLAGS = -lm


${PROJ} : dumpStats.o objects.o
	${CC} ${CFLAGS} dumpStats.o objects.o ${LDFLAGS} -o ${PROJ}

objects.o : objects.cpp objects.H
	${CC} ${CFLAGS} -c objects.cpp
	
dumpStats.o : dumpStats.cpp objects.H
	${CC} ${CFLAGS} -c dumpStats.cpp


clean:
	$(RM) *.o
	$(RM) $(PROJ)

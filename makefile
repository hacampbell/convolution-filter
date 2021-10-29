COMPILER = g++
CFLAGS = -Wall
EXES = convolution
CFILES = I R RI IR
all: ${EXES}

convolution:	convolution.cc matrix.o
	${COMPILER} ${CFLAGS} -pthread convolution.cc matrix.o -o convolution
	

getMatrix:   getMatrix.c matrix.o
	${COMPILER} ${CFLAGS} getMatrix.c matrix.o -o getMatrix


mkRandomMatrix:    mkRandomMatrix.c  matrix.o
	${COMPILER} ${CFLAGS} mkRandomMatrix.c matrix.o -o mkRandomMatrix


%.o: %.c %.h  makefile
	${COMPILER} ${CFLAGS} $< -c 

clean:
	rm -f *.o *~ ${EXES} ${CFILES}

run:
	./convolution test_matrix 1 5
SOURCES=$(wildcard *.cpp)

FLAGS=-g

all: main

main: $(SOURCES) Makefile
	mpic++ $(SOURCES) $(FLAGS) -o main

clear: clean

clean:
	rm main a.out

run: main Makefile ${SOURCES}
		mpirun -oversubscribe -np 8 ./main 4 4 2

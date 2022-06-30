F=-pthread
S=-lrt

all: util1_1 util1_2 util2

util1_1: util1_1.c
	gcc -o $@ $^ $(F) $(S)

util1_2: util1_2.c
	gcc -o $@ $^ $(F) $(S)

util2: util2.c
	gcc -o $@ $^ $(S)

clean:
	rm -f util1_1 util1_2 util2

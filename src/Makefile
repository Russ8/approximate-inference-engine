
PLATFORM := $(shell uname)

.PHONY:  clean

all : inference.bin$(EXT)

inference.bin$(EXT) : main.o $(LINK)
	g++ $(DEFS) -o inference.bin main.o $(LINK) $(GL_LIBS)

main.o : main.cpp $(LINK)
	g++ $(DEFS) -c main.cpp

clean:
	rm -f *.o inference.bin$(EXT)

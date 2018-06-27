CPP=g++
FLEXPP=flex++

COMMON_DEPS=Makefile

all: oemros

clean:
	rm -f turboini.flex.{cpp,h} *.o

oemros: main.o turboini.flex.o $(COMMON_DEPS)
	$(CPP) -o $@ *.o

%.o: %.cpp %.h $(COMMON_DEPS)
	$(CPP) -c -o $@ $< $(CFLAGS)

turboini.flex.cpp: turboini.flexpp $(COMMON_DEPS)
	$(FLEXPP) -o $@ --header-file=turboini.flex.h $<

CXX=g++
CXXFLAGS+=-Wall -Werror -MMD -ggdb
FLEX=flex

COMMON_DEPS=Makefile

OBJS=main.o turboini.flex.o
DEPS=${OBJS:.o=.d}

all: oemros

clean:
	rm -f turboini.flex.cpp *.o *.d

oemros: main.o turboini.flex.o $(COMMON_DEPS)
	$(CXX) -o $@ *.o

%.o: %.cpp %.h $(COMMON_DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

turboini.flex.cpp: turboini.flex $(COMMON_DEPS)
	$(FLEX) -o $@ $<

-include $(DEPS)

CXX=g++
CXXFLAGS+=-Wall -Werror -MMD -ggdb
FLEXPP=flex++

COMMON_DEPS=Makefile

OBJS=main.o turboini.flex.o
DEPS=${OBJS:.o=.d}

all: oemros

clean:
	rm -f turboini.flex.{cpp,h} *.o *.d

oemros: main.o turboini.flex.o | $(COMMON_DEPS)
	$(CXX) -o $@ *.o

main.o : turboini.flex.h

%.o: %.cpp %.h | $(COMMON_DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

turboini.flex.cpp turboini.flex.h: turboini.flexpp | $(COMMON_DEPS)
	$(FLEXPP) -o $@ --header-file=turboini.flex.h $<

-include $(DEPS)

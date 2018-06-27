CXX=g++
CXXFLAGS+=-Wall -Werror -MMD -ggdb
FLEXPP=flex++

COMMON_DEPS=Makefile

OBJS=main.o turboini.flex.o
DEPS=${OBJS:.o=.d}

OEMROS_BIN=oemros

all: oemros

clean:
	rm -f $(OEMROS_BIN) turboini.flex.cpp *.o *.d

$(OEMROS_BIN): main.o turboini.flex.o $(COMMON_DEPS)
	$(CXX) -o $@ *.o

%.o: %.cpp %.h $(COMMON_DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

turboini.flex.cpp: turboini.flexpp $(COMMON_DEPS)
	$(FLEXPP) -o $@ $<

-include $(DEPS)

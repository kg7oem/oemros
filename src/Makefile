CXX=g++
CXXFLAGS+=-std=c++11 -Wall -Werror -MMD -ggdb
FLEXPP=flex++

COMMON_DEPS=Makefile

OBJS=main.o turboini.flex.o
DEPS=${OBJS:.o=.d}

OEMROS_BIN=oemros

all: oemros

clean:
	rm -f $(OEMROS_BIN) turboini.flex.cpp *.o *.d

$(OEMROS_BIN): main.o turboini.flex.o $(COMMON_DEPS)
	$(CXX) $(CXXFLAGS) -o $@ *.o

%.o: %.cpp %.h $(COMMON_DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(CFLAGS)

turboini.flex.o : CXXFLAGS+=-Wno-sign-compare

turboini.flex.cpp: turboini.flexpp $(COMMON_DEPS)
	$(FLEXPP) -o $@ $<

-include $(DEPS)

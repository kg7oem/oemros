CXX=g++
CXXFLAGS+=-std=c++11 -Wall -Werror -MMD -ggdb
FLEXPP=flex++

COMMON_DEPS=Makefile

OBJS=main.o turboini.flex.o logging.o system.o runloop.o
DEPS=${OBJS:.o=.d}

OEMROS_BIN=oemros

all: $(OEMROS_BIN)

clean:
	rm -f $(OEMROS_BIN) turboini.flex.cpp $(OBJS) $(DEPS)

$(OEMROS_BIN): $(OBJS) $(COMMON_DEPS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp %.h $(COMMON_DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(CFLAGS)

turboini.flex.o: CXXFLAGS+=-Wno-sign-compare

turboini.flex.cpp: turboini.flexpp $(COMMON_DEPS)
	$(FLEXPP) -o $@ $<

-include $(DEPS)

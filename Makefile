PROG := synthctrl
SRCS := synthctrl.cpp io.cpp
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)
INCL := -I picojson -I rtmidi-2.1.0
CC := g++

all: $(PROG)

-include $(DEPS)

$(PROG): $(OBJS)
	$(CC) -lasound -o $@ $^

%.o: %.cpp
	$(CC) -c -MMD -MP $< $(INCL) 

clean:
	rm -f $(PROG) $(OBJS) $(DEPS)



PROG := synthctrl
SRCS := synthctrl.cpp io.cpp
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)

CC := g++

all: $(PROG)

-include $(DEPS)

$(PROG): $(OBJS)
	$(CC) -lasound -o $@ $^

%.o: %.cpp
	$(CC) -c -MMD -MP $< -I picojson

clean:
	rm -f $(PROG) $(OBJS) $(DEPS)



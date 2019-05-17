CC=gcc
FLAGS=#-Werror -Wall -pedantic
TARGET=logging-daemon
MODULES=linked_list
OBJS = $(addprefix obj/, $(addsuffix .o,$(MODULES)))

all: $(TARGET)

$(TARGET): main.c $(OBJS)
	$(CC) $(FLAGS) $(OBJS) main.c -o $@

obj/%.o : %.c %.h
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	rm -rf obj/ $(TARGET)

CC=gcc
FLAGS=#-Werror -Wall -pedantic
TARGET=main
MODULES=
OBJS = $(addprefix obj/, $(addsuffix .o,$(MODULES)))

all: $(TARGET)

$(TARGET): $(TARGET).c $(OBJS)
	$(CC) $(FLAGS) $(OBJS) $(TARGET).c -o $@

obj/%.o : %.c %.h
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	rm -rf obj/ $(TARGET)

INC = .
CFLAGS = -g -I$(INC) -Wall -Wextra
SFLAGS = -g -I$(INC) -Wall -Wextra -static
TARGET = ringbuffer ring_pointers ring_var 
#OBJS = src/commands.c util/heap.c util/list.c
all: $(TARGET)
ringbuffer: ringbuffer.c 
#	$(CC) $(CFLAGS) -o $@ $(OBJS) $<
	$(CC) $(CFLAGS) -o $@ $<
clean:
	$(RM) $(TARGET) 

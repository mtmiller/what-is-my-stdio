CC = gcc
CLEANFILES = what-is-my-stdio
SOURCES = what-is-my-stdio.c
OBJECTS = what-is-my-stdio.o

default: all

all: what-is-my-stdio

what-is-my-stdio: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	-rm -f $(CLEANFILES) *.o

.PHONY: all clean default

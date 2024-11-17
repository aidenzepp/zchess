CC = gcc
CFLAGS = $(shell pkg-config --cflags raylib) -O2 -Wall -Wextra
LDFLAGS = $(shell pkg-config --libs raylib) -lm

# If pkg-config fails, fall back to manual flags
ifeq ($(shell which pkg-config),)
	CFLAGS = -I/usr/local/include
	LDFLAGS = -L/usr/local/lib -lraylib -lm
endif

# Get all .c files from src directory
SRCS = $(wildcard src/*.c)
# Create list of object files
OBJS = $(SRCS:.c=.o)
TARGET = zchess

all: $(TARGET)

# Link all object files into final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile each .c file into .o file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean

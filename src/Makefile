TARGET = ww
CC = gcc
CFLAGS = -g -std=c99 -Wall -Werror -Wvla -Wextra -Wundef -Wshadow -Wpointer-arith -O2 -fsanitize=address,undefined


OBJ = $(TARGET).o

all: $(TARGET)

run: $(TARGET)
	./$<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET).o: $(TARGET).c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -rfv $(TARGET) *.o *.a *.dylib *.dSYM

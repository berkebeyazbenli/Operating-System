CC = gcc
CFLAGS = -Wall -pthread
TARGET = reader_writer

all: $(TARGET)

$(TARGET): reader_writer.c
	$(CC) $(CFLAGS) -o $(TARGET) reader_writer.c

clean:
	rm -f $(TARGET) 
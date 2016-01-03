SOURCES = test.cc
TARGET = ./test


CC = clang++
CFLAGS = -std=c++14 -Wall -g -m32 -fno-builtin -fno-exceptions -fno-stack-protector -fno-rtti -I ../include

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
	@echo test 
	$(TARGET)

debug:
	cgdb $(TARGET)

clean:
	rm $(TARGET)

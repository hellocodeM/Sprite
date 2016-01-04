#!Makefile

C_SOURCES = $(shell find lib/ init/ -name "*.cc")
C_OBJECTS = $(patsubst %.cc, %.o, $(C_SOURCES))
S_SOURCES = $(shell find . -name "*.s")
S_OBJECTS = $(patsubst %.s, %.o, $(S_SOURCES))
HEADERS = $(shell find . -name "*.h")
TARGET = Sprite

CC = clang++
LD = ld
ASM = nasm

C_FLAGS = -std=c++14 -c -Wall -Wno-unused-function -m32 -ggdb -fno-builtin -fno-exceptions -fno-stack-protector -fno-rtti -I include 
LD_FLAGS = -T scripts/kernel.ld -m elf_i386 -static -nostdlib
#C_FLAGS = -std=c++14 -c -Wall -Wno-unused-function -m32 -ggdb -gstabs+ -nostdinc -fno-builtin -fno-exceptions -fno-stack-protector -I include
#LD_FLAGS = -T scripts/kernel.ld -m elf_i386 -nostdlib
ASM_FLAGS = -f elf -g -F stabs

all: $(S_OBJECTS) $(C_OBJECTS) $(HEADERS) link

.cc.o:
	@echo 编译代码文件 $< ...
	$(CC) $(C_FLAGS) $< -o $@

.s.o:
	@echo 编译汇编文件 $< ...
	$(ASM) $(ASM_FLAGS) $<

link:
	@echo 链接内核文件...
	$(LD) $(LD_FLAGS) $(S_OBJECTS) $(C_OBJECTS) -o $(TARGET)

clean:
	rm $(S_OBJECTS) $(C_OBJECTS) $(TARGET) 

qemu:
	qemu  -kernel $(TARGET) -hda hdd.img

debug:
	qemu -S -gdb tcp::1080 -hda hdd.img -kernel $(TARGET) &
	cgdb -x scripts/gdbinit

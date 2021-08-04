all: BootLoader.bin

BootLoader.bin: src/BootLoader.asm
	nasm -o BootLoader.bin src/BootLoader.asm

clean:
	rm -f BootLoader.bin
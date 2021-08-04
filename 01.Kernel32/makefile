ASM32 = nasm
GPP32 = g++ -c -m32 -ffreestanding
LD32 = ld -melf_i386 -T ../linker.ld -nostdlib -e Main -Ttext 0x10200
OBJCOPY32 = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJDIR = tmp
SRCDIR = src

all: prepare Kernel32.bin

prepare:
	mkdir -p $(OBJDIR)

$(OBJDIR)/EntryPoint.bin: $(SRCDIR)/EntryPoint.s
	nasm -o $@ $<

dep:
	@echo [INFO] Make Dependancy File
	make -C $(OBJDIR) -f ../makefile InternalDependency
	@echo [INFO] Dependancy Search Complete

ExecuteInternelBuild: dep
	make -C $(OBJDIR) -f ../makefile Kernel32.elf

$(OBJDIR)/Kernel32.elf.bin: ExecuteInternelBuild
	$(OBJCOPY32) $(OBJDIR)/Kernel32.elf $@

Kernel32.bin: $(OBJDIR)/EntryPoint.bin $(OBJDIR)/Kernel32.elf.bin
	cat $^ > $@

clean:
	rm -r $(OBJDIR)
	rm -f Kernel32.bin

CENTRYPOINTOBJFILE = Main.o
CSRCFILES = $(wildcard ../$(SRCDIR)/*.cpp)
ASSEMBLYSRCFILES = $(wildcard ../$(SRCDIR)/*.asm)
COBJFILES = $(subst Main.o, , $(notdir $(patsubst %.cpp, %.o, $(CSRCFILES))))
ASSEMBLYOBJFILES = $(notdir $(patsubst %.asm, %.o, $(ASSEMBLYSRCFILES)))

%.o: ../$(SRCDIR)/%.cpp
	$(GPP32) -c $<

%.o: ../$(SRCDIR)/%.asm
	$(ASM32) -f elf32 -o $@ $<

InternalDependency:
	$(GPP32) -MM $(CSRCFILES) > Dependancy.dep

Kernel32.elf: $(CENTRYPOINTOBJFILE) $(COBJFILES) $(ASSEMBLYOBJFILES)
	$(LD32) -o $@ $^

ifeq (Dependency.dep, $(wildcard Dependancy.dep))
include Dependency.dep
endif
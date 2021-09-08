ASM64 = nasm -f elf64
GPP64 = g++ -c -m64 -ffreestanding -mmanual-endbr
LD64 = ld -melf_x86_64 -T ../linker.ld -nostdlib -e Main -Ttext 0x200000
OBJCOPY64 = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJDIR = tmp
SRCDIR = src

all: prepare Kernel64.bin

prepare:
	mkdir -p $(OBJDIR)

dep:
	@echo [INFO] Make Dependancy File
	make -C $(OBJDIR) -f ../makefile InternalDependency
	@echo [INFO] Dependancy Search Complete

ExecuteInternelBuild: dep
	make -C $(OBJDIR) -f ../makefile Kernel64.elf

$(OBJDIR)/Kernel64.elf.bin: ExecuteInternelBuild
	$(OBJCOPY64) $(OBJDIR)/Kernel64.elf $@

Kernel64.bin: $(OBJDIR)/Kernel64.elf.bin
	cat $^ > $@

clean:
	rm -r $(OBJDIR)
	rm -f Kernel64.bin

ENTRYPOINTSRCFILE = ../$(SRCDIR)/EntryPoint.s
ENTRYPOINTOBJFILE = EntryPoint.o
CSRCFILES = $(wildcard ../$(SRCDIR)/*.cpp)
ASSEMBLYSRCFILES = $(wildcard ../$(SRCDIR)/*.asm)
COBJFILES =  $(notdir $(patsubst %.cpp, %.o, $(CSRCFILES)))
ASSEMBLYOBJFILES = $(notdir $(patsubst %.asm, %.o, $(ASSEMBLYSRCFILES)))

$(ENTRYPOINTOBJFILE): $(ENTRYPOINTSRCFILE)
	$(ASM64) -o $@ $<

%.o: ../$(SRCDIR)/%.cpp
	$(GPP64) -c $<

%.o: ../$(SRCDIR)/%.asm
	$(ASM64) -o $@ $<

InternalDependency:
	$(GPP64) -MM $(CSRCFILES) > Dependancy.dep

Kernel64.elf: $(ENTRYPOINTOBJFILE) $(COBJFILES) $(ASSEMBLYOBJFILES)
	$(LD64) -o $@ $^
	objdump -d $@ > dump.txt

ifeq (Dependency.dep, $(wildcard Dependancy.dep))
include Dependency.dep
endif
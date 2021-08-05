all: BootLoader Kernel32 Kernel64 Disk.img

BootLoader:
	@echo [INFO] Build BootLoader Start
	make -C 00.BootLoader
	@echo [INFO] Build Complete

Kernel32:
	@echo [INFO] Build 32Bit Kernel Start
	make -C 01.Kernel32
	@echo [INFO] Build Complete

Kernel64:
	@echo [INFO] Build 64Bit Kernel Start
	make -C 02.Kernel64
	@echo [INFO] Build Complete

Disk.img: 00.BootLoader/BootLoader.bin 01.Kernel32/Kernel32.bin 02.Kernel64/Kernel64.bin
	@echo [INFO] Disk Image Build Start
	./ImageMaker $^
	@echo [INFO] All Build Complete

ImageMaker:
	make -C 04.Utility/00.ImageMaker

remove:
	rm -f ImageMaker

clean:
	make -C 00.BootLoader clean
	make -C 01.Kernel32 clean
	make -C 02.Kernel64 clean
	rm -f Disk.img

run:
	qemu-system-x86_64 -L . -m 64 -fda Disk.img -M pc
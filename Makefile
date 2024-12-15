CC = gcc
CCFLAGS = -Wall

SOURCES = fat.c
INCLUDES = fat.h

FAT12 = mkfs.fat -F 12
SECTOR_SIZE = 512
CLUSTERS = 2880
DRIVE = drive.iso

main: $(SOURCES)
	$(CC) $(CCFLAGS) -o $@ $<
	
drive:
	dd if=/dev/zero of=$(DRIVE) bs=$(SECTOR_SIZE) count=$(CLUSTERS)
	$(FAT12) -S $(SECTOR_SIZE) $(DRIVE)
	echo "HELLO WORLD FROM FAT12!" > hello.txt
	mcopy -i $(DRIVE) hello.txt "::hello.txt"
	rm -rf hello.txt

clean:
	rm -rf ./main

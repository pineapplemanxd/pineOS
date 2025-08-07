AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

CFLAGS = -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone
LDFLAGS = -m elf_i386 -T linker.ld

KERNEL_OBJS = multiboot_header.o kernel/kernel.o kernel/io.o kernel/memory.o kernel/process.o kernel/filesystem.o kernel/string.o kernel/storage.o kernel/user.o kernel/network.o kernel/pci.o kernel/netstack.o kernel/virtio_net.o kernel/e1000.o kernel/wifi_ax201.o kernel/amd_pcnet.o

.PHONY: all clean run

all: os.iso

multiboot_header.o: multiboot_header.asm
	$(AS) -f elf32 -o multiboot_header.o multiboot_header.asm

kernel/kernel.o: kernel/kernel.c kernel/kernel.h
	$(CC) $(CFLAGS) -c -o kernel/kernel.o kernel/kernel.c

kernel/io.o: kernel/io.c kernel/io.h
	$(CC) $(CFLAGS) -c -o kernel/io.o kernel/io.c

kernel/memory.o: kernel/memory.c kernel/memory.h
	$(CC) $(CFLAGS) -c -o kernel/memory.o kernel/memory.c

kernel/process.o: kernel/process.c kernel/process.h
	$(CC) $(CFLAGS) -c -o kernel/process.o kernel/process.c

kernel/filesystem.o: kernel/filesystem.c kernel/filesystem.h
	$(CC) $(CFLAGS) -c -o kernel/filesystem.o kernel/filesystem.c

kernel/string.o: kernel/string.c kernel/string.h
	$(CC) $(CFLAGS) -c -o kernel/string.o kernel/string.c

kernel/storage.o: kernel/storage.c kernel/storage.h
	$(CC) $(CFLAGS) -c -o kernel/storage.o kernel/storage.c

kernel/user.o: kernel/user.c kernel/user.h
	$(CC) $(CFLAGS) -c -o kernel/user.o kernel/user.c

kernel/network.o: kernel/network.c kernel/network.h
	$(CC) $(CFLAGS) -c -o kernel/network.o kernel/network.c

kernel/pci.o: kernel/pci.c kernel/pci.h
	$(CC) $(CFLAGS) -c -o kernel/pci.o kernel/pci.c

kernel/netstack.o: kernel/netstack.c kernel/netstack.h
	$(CC) $(CFLAGS) -c -o kernel/netstack.o kernel/netstack.c

kernel/virtio_net.o: kernel/virtio_net.c kernel/virtio_net.h
	$(CC) $(CFLAGS) -c -o kernel/virtio_net.o kernel/virtio_net.c

kernel/e1000.o: kernel/e1000.c kernel/e1000.h
	$(CC) $(CFLAGS) -c -o kernel/e1000.o kernel/e1000.c

kernel/wifi_ax201.o: kernel/wifi_ax201.c kernel/wifi_ax201.h
	$(CC) $(CFLAGS) -c -o kernel/wifi_ax201.o kernel/wifi_ax201.c

kernel/amd_pcnet.o: kernel/amd_pcnet.c kernel/amd_pcnet.h
	$(CC) $(CFLAGS) -c -o kernel/amd_pcnet.o kernel/amd_pcnet.c

kernel.elf: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o kernel.elf $(KERNEL_OBJS)

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary kernel.elf kernel.bin

os.iso: kernel.elf grub.cfg
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/kernel.elf
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o os.iso iso

run: os.iso
	qemu-system-i386 -cdrom os.iso -m 32

clean:
	rm -rf *.bin *.elf *.iso *.o iso kernel/*.o kernel/*.bin userlib/*.o 
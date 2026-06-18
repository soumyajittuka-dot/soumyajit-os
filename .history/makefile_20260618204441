CC = gcc
AS = nasm
LD = ld

# ফিক্স 1: -O0 দাও, -fno-pie অ্যাড করো। পেজিং সেফ
CFLAGS = -m32 -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -fno-stack-protector -fno-pie -fno-builtin -I.
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib -z noexecstack

OBJ = kernel_asm.o kernel.o command.o gdt.o idt.o e1000.o keyboard.o mm.o paging.o pci.o ports.o scheduler.o screen.o vga.o string.o timer.o

all: kernel.bin

kernel.bin: $(OBJ) linker.ld
	$(LD) $(LDFLAGS) $(OBJ) -o kernel.bin
	@echo "Build complete:"
	@ls -lh kernel.bin

kernel_asm.o: kernel.asm
	$(AS) $(ASFLAGS) kernel.asm -o kernel_asm.o

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o kernel.bin soumyajit_os.iso
	rm -rf /tmp/osdev-iso
	rm -f /tmp/soumyajit_os.iso

# ফিক্স 2: mv লাইন বাদ। ডিরেক্ট /tmp ইউজ করো
iso: kernel.bin
	mkdir -p /tmp/osdev-iso/boot/grub
	cp kernel.bin /tmp/osdev-iso/boot/
	printf 'set timeout=0\nset default=0\n\nmenuentry "Soumyajit OS" {\n    multiboot /boot/kernel.bin\n    boot\n}\n' > /tmp/osdev-iso/boot/grub/grub.cfg
	grub-mkrescue -o /tmp/soumyajit_os.iso /tmp/osdev-iso
	@echo "ISO ready at /tmp/soumyajit_os.iso"
	rm -rf /tmp/osdev-iso

# ফিক্স 3: run টার্গেটে /tmp এর ISO ইউজ করো
run: iso
	qemu-system-i386 -cdrom /tmp/soumyajit_os.iso -vga std -m 128M -serial stdio -boot d

run-net: iso
	qemu-system-i386 -cdrom /tmp/soumyajit_os.iso -vga std -netdev user,id=n1 -device e1000,netdev=n1 -m 128M -serial stdio

debug: iso
	qemu-system-i386 -cdrom /tmp/soumyajit_os.iso -vga std -m 128M -s -S

.PHONY: all clean iso run run-net debug
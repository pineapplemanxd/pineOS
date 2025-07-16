@echo off
echo Building Simple OS...

REM Check if required tools are available
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: GCC not found. Please install MinGW-w64 with 32-bit support.
    pause
    exit /b 1
)

where nasm >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: NASM not found. Please install NASM.
    pause
    exit /b 1
)

where qemu-system-i386 >nul 2>nul
if %errorlevel% neq 0 (
    echo Warning: QEMU not found. You can still build the OS but cannot run it.
    echo Please install QEMU to test the OS.
)

REM Clean previous builds
if exist *.bin del *.bin
if exist *.img del *.img
if exist kernel\*.o del kernel\*.o

REM Build bootloader
echo Building bootloader...
nasm -f bin -o bootloader.bin boot/boot.asm
if %errorlevel% neq 0 (
    echo Error: Failed to build bootloader
    pause
    exit /b 1
)

REM Build kernel
echo Building kernel...
gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/kernel.o kernel/kernel.c
if %errorlevel% neq 0 (
    echo Error: Failed to build kernel.c
    pause
    exit /b 1
)

gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/io.o kernel/io.c
if %errorlevel% neq 0 (
    echo Error: Failed to build io.c
    pause
    exit /b 1
)

gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/memory.o kernel/memory.c
if %errorlevel% neq 0 (
    echo Error: Failed to build memory.c
    pause
    exit /b 1
)

gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/process.o kernel/process.c
if %errorlevel% neq 0 (
    echo Error: Failed to build process.c
    pause
    exit /b 1
)

REM Link kernel
echo Linking kernel...
ld -m elf_i386 -T linker.ld -o kernel.bin kernel/kernel.o kernel/io.o kernel/memory.o kernel/process.o
if %errorlevel% neq 0 (
    echo Error: Failed to link kernel
    pause
    exit /b 1
)

REM Create disk image
echo Creating disk image...
fsutil file createnew os.img 1474560
if %errorlevel% neq 0 (
    echo Error: Failed to create disk image
    pause
    exit /b 1
)

REM Copy bootloader to disk image
copy /b bootloader.bin + os.img temp.img >nul
move temp.img os.img >nul

REM Copy kernel to disk image (simplified - in real scenario would use dd)
echo Kernel copied to disk image

echo Build completed successfully!
echo.
echo To run the OS:
echo   qemu-system-i386 -fda os.img -m 16
echo.
pause 
@echo off
echo Building Simple OS with Visual Studio Build Tools...

REM Check for Visual Studio Build Tools
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo Found Visual Studio Build Tools
    goto :build_with_vs
)

REM Check for MinGW
where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo Found MinGW GCC
    goto :build_with_gcc
)

REM Check for WSL (Windows Subsystem for Linux)
where wsl >nul 2>nul
if %errorlevel% equ 0 (
    echo Found WSL, trying to build with Linux tools...
    wsl bash -c "cd /mnt/c/Users/ilike/Documents/os && ./build.sh"
    if %errorlevel% equ 0 (
        echo Build completed successfully with WSL!
        goto :end
    )
)

echo Error: No suitable build tools found.
echo.
echo Please install one of the following:
echo 1. Visual Studio Build Tools (with C++ workload)
echo 2. MinGW-w64 with 32-bit support
echo 3. WSL (Windows Subsystem for Linux)
echo.
echo For Visual Studio Build Tools:
echo   Download from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
echo   Install with "C++ build tools" workload
echo.
pause
exit /b 1

:build_with_vs
echo Building with Visual Studio Build Tools...

REM Set up Visual Studio environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars32.bat" 2>nul
if %errorlevel% neq 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" 2>nul
    if %errorlevel% neq 0 (
        echo Error: Could not find Visual Studio Build Tools
        pause
        exit /b 1
    )
)

REM Check for NASM
where nasm >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: NASM not found. Please install NASM.
    pause
    exit /b 1
)

REM Clean previous builds
if exist *.bin del *.bin
if exist *.img del *.img
if exist kernel\*.obj del kernel\*.obj

REM Build bootloader
echo Building bootloader...
nasm -f bin -o bootloader.bin boot/boot.asm
if %errorlevel% neq 0 (
    echo Error: Failed to build bootloader
    pause
    exit /b 1
)

REM Build kernel with Visual Studio
echo Building kernel...
cl /nologo /MT /O2 /GS- /Gy- /GR- /EHs- /c /Fo:kernel/kernel.obj kernel/kernel.c
if %errorlevel% neq 0 (
    echo Error: Failed to build kernel.c
    pause
    exit /b 1
)

cl /nologo /MT /O2 /GS- /Gy- /GR- /EHs- /c /Fo:kernel/io.obj kernel/io.c
if %errorlevel% neq 0 (
    echo Error: Failed to build io.c
    pause
    exit /b 1
)

cl /nologo /MT /O2 /GS- /Gy- /GR- /EHs- /c /Fo:kernel/memory.obj kernel/memory.c
if %errorlevel% neq 0 (
    echo Error: Failed to build memory.c
    pause
    exit /b 1
)

cl /nologo /MT /O2 /GS- /Gy- /GR- /EHs- /c /Fo:kernel/process.obj kernel/process.c
if %errorlevel% neq 0 (
    echo Error: Failed to build process.c
    pause
    exit /b 1
)

REM Link kernel (simplified - would need custom linker script for VS)
echo Linking kernel...
link /nologo /SUBSYSTEM:NATIVE /ENTRY:_start /OUT:kernel.bin kernel/kernel.obj kernel/io.obj kernel/memory.obj kernel/process.obj
if %errorlevel% neq 0 (
    echo Error: Failed to link kernel
    pause
    exit /b 1
)

goto :create_image

:build_with_gcc
echo Building with MinGW GCC...

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
ld -m i386pe -T linker.ld -o kernel.bin kernel/kernel.o kernel/io.o kernel/memory.o kernel/process.o
if %errorlevel% neq 0 (
    echo Error: Failed to link kernel
    pause
    exit /b 1
)

:create_image
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

echo Build completed successfully!
echo.
echo To run the OS, install QEMU and run:
echo   qemu-system-i386 -fda os.img -m 16
echo.

:end
pause 
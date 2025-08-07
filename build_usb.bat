@echo off
REM Build script for creating USB-bootable pineOS image on Windows

echo Building pineOS for USB boot...

REM Clean previous builds
make clean

REM Build the OS
make os.iso

if not exist "os.iso" (
    echo Error: Failed to build os.iso
    exit /b 1
)

echo.
echo USB image creation on Windows requires additional tools:
echo.
echo 1. Install QEMU for Windows
echo 2. Use Rufus or similar tool to create bootable USB
echo.
echo Steps to create USB boot drive:
echo 1. Download Rufus from https://rufus.ie/
echo 2. Insert your USB drive
echo 3. Run Rufus and select your USB drive
echo 4. Select "FreeDOS" or "Non bootable" 
echo 5. Click "SELECT" and choose the os.iso file
echo 6. Click "START" to create the bootable USB
echo.
echo Alternative: Use the os.iso directly with virtualization software
echo.
echo To test in QEMU (if installed):
echo   qemu-system-i386 -cdrom os.iso -m 32
echo.
echo The OS now includes persistent filesystem support!
echo Use these commands in the OS:
echo   storage  - List storage devices
echo   save     - Save filesystem to USB
echo   load     - Load filesystem from USB  
echo   format   - Format USB device
echo.
pause
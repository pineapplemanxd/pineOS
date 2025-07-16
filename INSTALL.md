# Installation Guide for Windows

This guide will help you set up the development environment to build and run the simple operating system.

## Option 1: Quick Setup with Chocolatey (Recommended)

If you have Chocolatey installed, this is the fastest way:

1. **Install Chocolatey** (if not already installed):
   ```powershell
   Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
   ```

2. **Install required tools**:
   ```powershell
   choco install mingw nasm qemu
   ```

3. **Restart your terminal** and try building:
   ```powershell
   .\build.bat
   ```

## Option 2: Manual Installation

### Step 1: Install MinGW-w64 (GCC Compiler)

1. **Download MSYS2** from: https://www.msys2.org/
2. **Install MSYS2** (usually to `C:\msys64`)
3. **Open MSYS2 terminal** and run:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-nasm
   ```
4. **Add to PATH**: Add `C:\msys64\mingw64\bin` to your system PATH

### Step 2: Install NASM (Assembler)

1. **Download NASM** from: https://www.nasm.us/
2. **Install NASM** and add to PATH
3. **Verify installation**:
   ```powershell
   nasm --version
   ```

### Step 3: Install QEMU (Emulator)

1. **Download QEMU** from: https://www.qemu.org/download/#windows
2. **Install QEMU** and add to PATH
3. **Verify installation**:
   ```powershell
   qemu-system-i386 --version
   ```

## Option 3: Using Visual Studio Build Tools

If you prefer Microsoft's tools:

1. **Download Visual Studio Build Tools** from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
2. **Install with "C++ build tools" workload**
3. **Use the alternative build script**:
   ```powershell
   .\build_vs.bat
   ```

## Option 4: Using WSL (Windows Subsystem for Linux)

If you have WSL installed:

1. **Install WSL** (if not already installed):
   ```powershell
   wsl --install
   ```

2. **Install required packages in WSL**:
   ```bash
   sudo apt update
   sudo apt install gcc-multilib nasm qemu-system-x86 make
   ```

3. **Build using the Linux script**:
   ```powershell
   wsl bash -c "cd /mnt/c/Users/ilike/Documents/os && ./build.sh"
   ```

## Verification

After installation, verify that all tools are available:

```powershell
# Check GCC
gcc --version

# Check NASM
nasm --version

# Check QEMU
qemu-system-i386 --version
```

## Building the OS

Once all tools are installed, you can build the OS:

```powershell
# Using the main build script
.\build.bat

# Or using the alternative script
.\build_vs.bat
```

## Running the OS

After successful build, run the OS:

```powershell
qemu-system-i386 -fda os.img -m 16
```

## Troubleshooting

### "gcc: command not found"
- Make sure MinGW-w64 is installed and added to PATH
- Try restarting your terminal after installation

### "nasm: command not found"
- Install NASM and add to PATH
- Download from: https://www.nasm.us/

### "qemu-system-i386: command not found"
- Install QEMU and add to PATH
- Download from: https://www.qemu.org/download/#windows

### Build errors
- Make sure you're using 32-bit tools (gcc -m32)
- Check that all dependencies are installed
- Try cleaning and rebuilding: `make clean && make`

### Permission errors
- Run PowerShell as Administrator if needed
- Check that you have write permissions in the project directory

## Alternative: Online Development

If you can't install the tools locally, you can use online development environments:

1. **GitHub Codespaces** (if you have GitHub Pro)
2. **GitPod** (free tier available)
3. **Replit** (free tier available)

These environments usually have the required tools pre-installed.

## Next Steps

Once you have the development environment set up:

1. **Build the OS**: `.\build.bat`
2. **Run the OS**: `qemu-system-i386 -fda os.img -m 16`
3. **Explore the code**: Start with `kernel/kernel.c`
4. **Modify and extend**: Add new features to the OS

Happy coding! 🚀 
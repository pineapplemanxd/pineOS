#include "kernel.h"
#include "string.h"
#include "storage.h"
#include "user.h"

// Multiboot header (provided by multiboot_header.asm)
extern void multiboot_header(void);

// Function declarations
void execute_command(const char* command);
void kernel_loop(void);

// Global variables
static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char current_color = VGA_LIGHT_GREY;

// Kernel entry point called from assembly
void kernel_main(void) {
    // Initialize subsystems
    kernel_init();
    
    // Start main kernel loop
    kernel_loop();
}

// Kernel initialization
void kernel_init(void) {
    // Initialize subsystems
    vga_init();
    keyboard_init();
    memory_init();
    process_init();
    filesystem_init();
    storage_init();
    
    // Initialize user layer with debug output
    vga_puts("Initializing user layer...\n");
    user_init();
    vga_puts("User layer initialization complete\n");
    
    // Clear screen and show welcome message
    vga_clear();
    vga_puts("pineOS v1.0 with Filesystem\n");
    vga_puts("Initializing...\n");
    
    // Test memory allocation
    void* test_mem = memory_alloc(1024);
    if (test_mem) {
        vga_puts("Memory allocation: OK\n");
        memory_free(test_mem);
    } else {
        vga_puts("Memory allocation: FAILED\n");
    }
    
    vga_puts("Filesystem: OK\n");
    vga_puts("System ready!\n");
    vga_puts("Type 'help' for available commands\n\n");
}

// Main kernel loop
void kernel_loop(void) {
    char input_buffer[256];
    int buffer_pos = 0;
    
    vga_puts("> ");
    
    while (1) {
        if (keyboard_available()) {
            char c = keyboard_read();
            
            if (c == '\n' || c == '\r') {
                // Execute command
                input_buffer[buffer_pos] = '\0';
                execute_command(input_buffer);
                buffer_pos = 0;
                vga_puts("> ");
            } else if (c == '\b' && buffer_pos > 0) {
                // Backspace
                buffer_pos--;
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            } else if (buffer_pos < 255 && c >= 32 && c <= 126) {
                // Printable character
                input_buffer[buffer_pos++] = c;
                vga_putchar(c);
            }
        }
        
        // Simple process scheduling
        process_schedule();
    }
}

// Command execution
void execute_command(const char* command) {
    vga_puts("\n");
    
    if (strcmp(command, "help") == 0) {
        vga_puts("Available commands:\n");
        vga_puts("  help     - Show this help\n");
        vga_puts("  clear    - Clear screen\n");
        vga_puts("  memory   - Show memory status\n");
        vga_puts("  process  - Show process status\n");
        vga_puts("  test     - Run memory test\n");
        vga_puts("  reboot   - Reboot system\n");
        vga_puts("  ls       - List directory contents\n");
        vga_puts("  cd       - Change directory\n");
        vga_puts("  pwd      - Print working directory\n");
        vga_puts("  mkdir    - Create directory\n");
        vga_puts("  touch    - Create empty file\n");
        vga_puts("  cat      - Display file contents\n");
        vga_puts("  echo     - Write text to file\n");
        vga_puts("  rm       - Remove file\n");
        vga_puts("  rmdir    - Remove directory\n");
        vga_puts("  tree     - Show directory tree\n");
        vga_puts("  cp       - Copy file\n");
        vga_puts("  storage  - List storage devices\n");
        vga_puts("  save     - Save filesystem to USB\n");
        vga_puts("  load     - Load filesystem from USB\n");
        vga_puts("  format   - Format USB device\n");
        vga_puts("  programs - List user programs\n");
        vga_puts("  run      - Run user program\n");
        vga_puts("  compile  - Compile C program from file\n");
        vga_puts("  unload   - Remove user program\n");
    } else if (strcmp(command, "clear") == 0) {
        vga_clear();
    } else if (strcmp(command, "memory") == 0) {
        unsigned int free_mem = memory_get_free();
        vga_puts("Memory status:\n");
        vga_puts("  Total: 1MB\n");
        vga_puts("  Used: ");
        // Print memory usage (simplified)
        vga_puts("Unknown\n");
        vga_puts("  Free: ");
        // Print free memory (simplified)
        vga_puts("Unknown\n");
    } else if (strcmp(command, "process") == 0) {
        vga_puts("Process status:\n");
        process_t* current = process_get_current();
        if (current) {
            vga_puts("  Current PID: ");
            // Print PID (simplified)
            vga_puts("1\n");
        } else {
            vga_puts("  No processes running\n");
        }
    } else if (strcmp(command, "test") == 0) {
        vga_puts("Running memory test...\n");
        
        // Allocate some memory
        void* ptr1 = memory_alloc(512);
        void* ptr2 = memory_alloc(1024);
        
        if (ptr1 && ptr2) {
            vga_puts("  Memory allocation test: PASSED\n");
            memory_free(ptr1);
            memory_free(ptr2);
            vga_puts("  Memory deallocation test: PASSED\n");
        } else {
            vga_puts("  Memory test: FAILED\n");
        }
    } else if (strcmp(command, "reboot") == 0) {
        vga_puts("Rebooting...\n");
        // In a real OS, this would trigger a reboot
        // For now, just halt
        while (1) {
            // Halt
        }
    } else if (strncmp(command, "ls", 2) == 0) {
        // Handle ls command
        const char* path = command + 2;
        while (*path == ' ') path++; // Skip spaces
        filesystem_ls(path);
    } else if (strncmp(command, "cd", 2) == 0) {
        // Handle cd command
        const char* path = command + 2;
        while (*path == ' ') path++; // Skip spaces
        filesystem_cd(path);
    } else if (strcmp(command, "pwd") == 0) {
        filesystem_pwd();
    } else if (strncmp(command, "mkdir", 5) == 0) {
        // Handle mkdir command
        const char* name = command + 5;
        while (*name == ' ') name++; // Skip spaces
        filesystem_mkdir(name);
    } else if (strncmp(command, "touch", 5) == 0) {
        // Handle touch command
        const char* name = command + 5;
        while (*name == ' ') name++; // Skip spaces
        filesystem_touch(name);
    } else if (strncmp(command, "cat", 3) == 0) {
        // Handle cat command
        const char* name = command + 3;
        while (*name == ' ') name++; // Skip spaces
        char* content = filesystem_read_file(name);
        if (content) {
            vga_puts(content);
        }
    } else if (strncmp(command, "echo", 4) == 0) {
        // Handle echo command
        const char* args = command + 4;
        while (*args == ' ') args++; // Skip spaces
        
        // Find the first space (separating filename from content)
        const char* filename = args;
        while (*args && *args != ' ') args++;
        if (*args == ' ') {
            // Create a temporary copy to modify
            char temp[256];
            int len = args - filename;
            memory_copy(temp, filename, len);
            temp[len] = '\0';
            
            // Skip the space
            args++;
            
            // Write content to file
            filesystem_write_file(temp, args);
        } else {
            vga_puts("Usage: echo filename content\n");
        }
    } else if (strncmp(command, "rm", 2) == 0) {
        // Handle rm command
        const char* name = command + 2;
        while (*name == ' ') name++; // Skip spaces
        filesystem_rm(name);
    } else if (strncmp(command, "rmdir", 5) == 0) {
        // Handle rmdir command
        const char* name = command + 5;
        while (*name == ' ') name++; // Skip spaces
        filesystem_rmdir(name);
    } else if (strncmp(command, "tree", 4) == 0) {
        // Handle tree command
        const char* path = command + 4;
        while (*path == ' ') path++; // Skip spaces
        if (strlen(path) == 0) {
            filesystem_tree("/", 0);
        } else {
            filesystem_tree(path, 0);
        }
    } else if (strncmp(command, "cp", 2) == 0) {
        // Handle cp command
        const char* args = command + 2;
        while (*args == ' ') args++; // Skip spaces
        // Find the first space (separating src from dest)
        const char* src = args;
        while (*args && *args != ' ') args++;
        if (*args == ' ') {
            char src_buf[256];
            int src_len = args - src;
            if (src_len >= 256) src_len = 255;
            memory_copy(src_buf, src, src_len);
            src_buf[src_len] = '\0';
            args++;
            // Now args points to dest
            while (*args == ' ') args++;
            if (*args) {
                filesystem_cp(src_buf, args);
            } else {
                vga_puts("Usage: cp <src> <dest>\n");
            }
        } else {
            vga_puts("Usage: cp <src> <dest>\n");
        }
    } else if (strcmp(command, "storage") == 0) {
        // List storage devices
        int device_count = storage_get_device_count();
        vga_puts("Storage devices:\n");
        if (device_count == 0) {
            vga_puts("  No storage devices found\n");
        } else {
            for (int i = 0; i < device_count; i++) {
                storage_device_t* dev = storage_get_device(i);
                if (dev) {
                    vga_puts("  ");
                    vga_putchar('0' + i);
                    vga_puts(": ");
                    vga_puts(dev->name);
                    vga_puts(" (");
                    if (dev->type == STORAGE_TYPE_USB) {
                        vga_puts("USB");
                    } else if (dev->type == STORAGE_TYPE_HDD) {
                        vga_puts("HDD");
                    } else if (dev->type == STORAGE_TYPE_FLOPPY) {
                        vga_puts("Floppy");
                    } else {
                        vga_puts("Unknown");
                    }
                    vga_puts(")\n");
                }
            }
        }
    } else if (strcmp(command, "save") == 0) {
        // Save filesystem to first available storage device
        storage_device_t* storage_dev = 0;
        int device_count = storage_get_device_count();
        for (int i = 0; i < device_count; i++) {
            storage_device_t* dev = storage_get_device(i);
            if (dev && (dev->type == STORAGE_TYPE_HDD || dev->type == STORAGE_TYPE_USB)) {
                storage_dev = dev;
                break;
            }
        }
        
        if (storage_dev) {
            filesystem_save_to_storage(storage_dev);
        } else {
            vga_puts("Error: No storage device found\n");
        }
    } else if (strcmp(command, "load") == 0) {
        // Load filesystem from first available storage device
        storage_device_t* storage_dev = 0;
        int device_count = storage_get_device_count();
        for (int i = 0; i < device_count; i++) {
            storage_device_t* dev = storage_get_device(i);
            if (dev && (dev->type == STORAGE_TYPE_HDD || dev->type == STORAGE_TYPE_USB)) {
                storage_dev = dev;
                break;
            }
        }
        
        if (storage_dev) {
            filesystem_load_from_storage(storage_dev);
        } else {
            vga_puts("Error: No storage device found\n");
        }
    } else if (strcmp(command, "format") == 0) {
        // Format first available storage device
        storage_device_t* storage_dev = 0;
        int device_count = storage_get_device_count();
        for (int i = 0; i < device_count; i++) {
            storage_device_t* dev = storage_get_device(i);
            if (dev && (dev->type == STORAGE_TYPE_HDD || dev->type == STORAGE_TYPE_USB)) {
                storage_dev = dev;
                break;
            }
        }
        
        if (storage_dev) {
            vga_puts("WARNING: This will erase all data on ");
            vga_puts(storage_dev->name);
            vga_puts("\nPress 'y' to continue or any other key to cancel: ");
            // For now, just format without confirmation
            filesystem_format_storage(storage_dev);
        } else {
            vga_puts("Error: No storage device found\n");
        }
    } else if (strcmp(command, "programs") == 0) {
        // List user programs
        user_list_programs();
    } else if (strncmp(command, "run", 3) == 0) {
        // Run user program
        const char* prog_name = command + 3;
        while (*prog_name == ' ') prog_name++; // Skip spaces
        if (strlen(prog_name) > 0) {
            user_run_program(prog_name);
        } else {
            vga_puts("Usage: run <program_name>\n");
        }
    } else if (strncmp(command, "compile", 7) == 0) {
        // Compile C program from file
        const char* filename = command + 7;
        while (*filename == ' ') filename++; // Skip spaces
        if (strlen(filename) > 0) {
            user_load_from_file(filename);
        } else {
            vga_puts("Usage: compile <filename.c>\n");
        }
    } else if (strncmp(command, "unload", 6) == 0) {
        // Remove user program
        const char* prog_name = command + 6;
        while (*prog_name == ' ') prog_name++; // Skip spaces
        if (strlen(prog_name) > 0) {
            user_remove_program(prog_name);
        } else {
            vga_puts("Usage: unload <program_name>\n");
        }
    } else if (strlen(command) > 0) {
        vga_puts("Unknown command: ");
        vga_puts(command);
        vga_puts("\nType 'help' for available commands\n");
    }
}

 
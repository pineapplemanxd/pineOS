#include "filesystem.h"
#include "io.h"
#include "string.h"
#include "storage.h"

// Global filesystem instance
filesystem_t fs;

// Initialize filesystem
void filesystem_init(void) {
    // Initialize filesystem structure
    fs.next_entry = 0;
    
    // Create root directory
    fs.root = filesystem_create_file("/", FILE_TYPE_DIR);
    fs.current_dir = fs.root;
    
    // Create some default directories and files
    filesystem_mkdir("bin");
    filesystem_mkdir("home");
    filesystem_mkdir("etc");
    filesystem_mkdir("tmp");
    
    // Create some default files
    filesystem_write_file("/etc/version", "pineOS v1.0\n");
    filesystem_write_file("/etc/motd", "Welcome to pineOS!\n");
    filesystem_write_file("/home/readme.txt", "This is your home directory.\n");
}

// Create a new file entry
file_entry_t* filesystem_create_file(const char* name, int type) {
    if (fs.next_entry >= MAX_FILES + MAX_DIRS) {
        return 0; // No more space
    }
    
    file_entry_t* entry = &fs.entries[fs.next_entry++];
    
    // Initialize entry
    int name_len = strlen(name);
    if (name_len >= MAX_FILENAME) name_len = MAX_FILENAME - 1;
    memory_copy(entry->name, name, name_len);
    entry->name[name_len] = '\0';
    
    entry->type = type;
    entry->size = 0;
    entry->data = 0;
    entry->parent = 0;
    entry->children = 0;
    entry->next = 0;
    entry->used = 1;
    
    return entry;
}

// Find a file by path
file_entry_t* filesystem_find_file(const char* path) {
    if (strcmp(path, "/") == 0) {
        return fs.root;
    }
    
    if (strcmp(path, ".") == 0) {
        return fs.current_dir;
    }
    
    if (strcmp(path, "..") == 0) {
        return fs.current_dir->parent ? fs.current_dir->parent : fs.root;
    }
    
    // Handle relative paths
    file_entry_t* search_dir = fs.current_dir;
    if (path[0] == '/') {
        search_dir = fs.root;
        path++; // Skip leading slash
    }
    
    // Split path into components
    char path_copy[MAX_PATH];
    memory_copy(path_copy, path, strlen(path) + 1);
    
    char* component = strtok(path_copy, "/");
    while (component) {
        file_entry_t* found = 0;
        file_entry_t* child = search_dir->children;
        
        while (child) {
            if (strcmp(child->name, component) == 0) {
                found = child;
                break;
            }
            child = child->next;
        }
        
        if (!found) {
            return 0; // Not found
        }
        
        search_dir = found;
        component = strtok(0, "/");
    }
    
    return search_dir;
}

// Create a directory
int filesystem_mkdir(const char* name) {
    file_entry_t* parent = fs.current_dir;
    
    // Check if it's an absolute path
    if (name[0] == '/') {
        parent = fs.root;
        name++; // Skip leading slash
    }
    
    // Check if directory already exists
    file_entry_t* existing = filesystem_find_file(name);
    if (existing) {
        vga_puts("Error: Directory already exists\n");
        return -1;
    }
    
    // Create new directory
    file_entry_t* new_dir = filesystem_create_file(name, FILE_TYPE_DIR);
    if (!new_dir) {
        vga_puts("Error: Cannot create directory\n");
        return -1;
    }
    
    // Add to parent's children
    new_dir->parent = parent;
    new_dir->next = parent->children;
    parent->children = new_dir;
    
    vga_puts("Directory created: ");
    vga_puts(name);
    vga_puts("\n");
    return 0;
}

// Create an empty file
int filesystem_touch(const char* name) {
    file_entry_t* parent = fs.current_dir;
    
    // Check if it's an absolute path
    if (name[0] == '/') {
        parent = fs.root;
        name++; // Skip leading slash
    }
    
    // Check if file already exists
    file_entry_t* existing = filesystem_find_file(name);
    if (existing) {
        vga_puts("Error: File already exists\n");
        return -1;
    }
    
    // Create new file
    file_entry_t* new_file = filesystem_create_file(name, FILE_TYPE_FILE);
    if (!new_file) {
        vga_puts("Error: Cannot create file\n");
        return -1;
    }
    
    // Allocate data buffer
    new_file->data = memory_alloc(MAX_FILE_SIZE);
    if (!new_file->data) {
        vga_puts("Error: Out of memory\n");
        return -1;
    }
    
    // Add to parent's children
    new_file->parent = parent;
    new_file->next = parent->children;
    parent->children = new_file;
    
    vga_puts("File created: ");
    vga_puts(name);
    vga_puts("\n");
    return 0;
}

// Write content to a file
int filesystem_write_file(const char* name, const char* content) {
    file_entry_t* file = filesystem_find_file(name);
    
    if (!file) {
        // Create file if it doesn't exist
        if (filesystem_touch(name) != 0) {
            return -1;
        }
        file = filesystem_find_file(name);
    }
    
    if (file->type != FILE_TYPE_FILE) {
        vga_puts("Error: Not a file\n");
        return -1;
    }
    
    int content_len = strlen(content);
    if (content_len >= MAX_FILE_SIZE) {
        content_len = MAX_FILE_SIZE - 1;
    }
    
    memory_copy(file->data, content, content_len);
    file->data[content_len] = '\0';
    file->size = content_len;
    
    return 0;
}

// Read content from a file
char* filesystem_read_file(const char* name) {
    file_entry_t* file = filesystem_find_file(name);
    
    if (!file) {
        vga_puts("Error: File not found\n");
        return 0;
    }
    
    if (file->type != FILE_TYPE_FILE) {
        vga_puts("Error: Not a file\n");
        return 0;
    }
    
    return file->data;
}

// List directory contents
int filesystem_ls(const char* path) {
    file_entry_t* dir = fs.current_dir;
    
    if (path && strlen(path) > 0) {
        dir = filesystem_find_file(path);
    }
    
    if (!dir) {
        vga_puts("Error: Directory not found\n");
        return -1;
    }
    
    if (dir->type != FILE_TYPE_DIR) {
        vga_puts("Error: Not a directory\n");
        return -1;
    }
    
    vga_puts("Contents of ");
    vga_puts(dir->name);
    vga_puts(":\n");
    
    file_entry_t* child = dir->children;
    if (!child) {
        vga_puts("  (empty)\n");
        return 0;
    }
    
    while (child) {
        vga_puts("  ");
        if (child->type == FILE_TYPE_DIR) {
            vga_puts("[DIR] ");
        } else {
            vga_puts("      ");
        }
        vga_puts(child->name);
        if (child->type == FILE_TYPE_FILE) {
            vga_puts(" (");
            // Convert size to string
            char size_str[16];
            int size = child->size;
            int i = 0;
            do {
                size_str[i++] = '0' + (size % 10);
                size /= 10;
            } while (size > 0);
            for (int j = i - 1; j >= 0; j--) {
                vga_putchar(size_str[j]);
            }
            vga_puts(" bytes)");
        }
        vga_puts("\n");
        child = child->next;
    }
    
    return 0;
}

// Change directory
int filesystem_cd(const char* path) {
    if (!path || strlen(path) == 0) {
        fs.current_dir = fs.root;
        return 0;
    }
    
    file_entry_t* new_dir = filesystem_find_file(path);
    
    if (!new_dir) {
        vga_puts("Error: Directory not found\n");
        return -1;
    }
    
    if (new_dir->type != FILE_TYPE_DIR) {
        vga_puts("Error: Not a directory\n");
        return -1;
    }
    
    fs.current_dir = new_dir;
    return 0;
}

// Print working directory
int filesystem_pwd(void) {
    char path[MAX_PATH] = "";
    file_entry_t* current = fs.current_dir;
    
    // Build path from current directory to root
    while (current && current != fs.root) {
        char temp[MAX_PATH];
        memory_copy(temp, "/", 1);
        memory_copy(temp + 1, current->name, strlen(current->name));
        memory_copy(temp + 1 + strlen(current->name), path, strlen(path));
        memory_copy(path, temp, strlen(temp) + 1);
        current = current->parent;
    }
    
    if (strlen(path) == 0) {
        memory_copy(path, "/", 2);
    }
    
    vga_puts(path);
    vga_puts("\n");
    return 0;
}

// Remove a file
int filesystem_rm(const char* name) {
    file_entry_t* file = filesystem_find_file(name);
    
    if (!file) {
        vga_puts("Error: File not found\n");
        return -1;
    }
    
    if (file->type != FILE_TYPE_FILE) {
        vga_puts("Error: Not a file\n");
        return -1;
    }
    
    // Remove from parent's children list
    file_entry_t* parent = file->parent;
    if (parent->children == file) {
        parent->children = file->next;
    } else {
        file_entry_t* sibling = parent->children;
        while (sibling && sibling->next != file) {
            sibling = sibling->next;
        }
        if (sibling) {
            sibling->next = file->next;
        }
    }
    
    // Free file data
    if (file->data) {
        memory_free(file->data);
    }
    
    // Mark as unused
    file->used = 0;
    
    vga_puts("File removed: ");
    vga_puts(name);
    vga_puts("\n");
    return 0;
}

// Remove a directory (only if empty)
int filesystem_rmdir(const char* name) {
    file_entry_t* dir = filesystem_find_file(name);
    
    if (!dir) {
        vga_puts("Error: Directory not found\n");
        return -1;
    }
    
    if (dir->type != FILE_TYPE_DIR) {
        vga_puts("Error: Not a directory\n");
        return -1;
    }
    
    if (dir == fs.root) {
        vga_puts("Error: Cannot remove root directory\n");
        return -1;
    }
    
    if (dir->children) {
        vga_puts("Error: Directory not empty\n");
        return -1;
    }
    
    // Remove from parent's children list
    file_entry_t* parent = dir->parent;
    if (parent->children == dir) {
        parent->children = dir->next;
    } else {
        file_entry_t* sibling = parent->children;
        while (sibling && sibling->next != dir) {
            sibling = sibling->next;
        }
        if (sibling) {
            sibling->next = dir->next;
        }
    }
    
    // Mark as unused
    dir->used = 0;
    
    vga_puts("Directory removed: ");
    vga_puts(name);
    vga_puts("\n");
    return 0;
}

// Show directory tree
void filesystem_tree(const char* path, int depth) {
    file_entry_t* dir = fs.current_dir;
    
    if (path && strlen(path) > 0) {
        dir = filesystem_find_file(path);
    }
    
    if (!dir || dir->type != FILE_TYPE_DIR) {
        return;
    }
    
    // Print indentation
    for (int i = 0; i < depth; i++) {
        vga_puts("  ");
    }
    
    vga_puts(dir->name);
    vga_puts("/\n");
    
    // Print children
    file_entry_t* child = dir->children;
    while (child) {
        for (int i = 0; i < depth + 1; i++) {
            vga_puts("  ");
        }
        
        if (child->type == FILE_TYPE_DIR) {
            vga_puts(child->name);
            vga_puts("/\n");
            filesystem_tree(child->name, depth + 2);
        } else {
            vga_puts(child->name);
            vga_puts("\n");
        }
        
        child = child->next;
    }
} 

// Copy a file from src to dest
int filesystem_cp(const char* src, const char* dest) {
    file_entry_t* src_file = filesystem_find_file(src);
    if (!src_file) {
        vga_puts("Error: Source file not found\n");
        return -1;
    }
    if (src_file->type != FILE_TYPE_FILE) {
        vga_puts("Error: Source is not a file\n");
        return -1;
    }
    // Read source data
    char* src_data = src_file->data;
    int src_size = src_file->size;
    if (!src_data) {
        vga_puts("Error: Source file is empty\n");
        return -1;
    }
    // If dest exists and is a directory, error
    file_entry_t* dest_file = filesystem_find_file(dest);
    if (dest_file && dest_file->type == FILE_TYPE_DIR) {
        vga_puts("Error: Destination is a directory\n");
        return -1;
    }
    // Write to destination (creates file if needed)
    int result = filesystem_write_file(dest, src_data);
    if (result == 0) {
        vga_puts("File copied: ");
        vga_puts(src);
        vga_puts(" -> ");
        vga_puts(dest);
        vga_puts("\n");
    }
    return result;
} 

// Simple filesystem format for storage
// Sector 0: Filesystem header
// Sector 1+: File entries and data

typedef struct fs_header {
    char magic[8];           // "PINEFS\0\0"
    uint32_t version;        // Filesystem version
    uint32_t total_entries;  // Number of file entries
    uint32_t data_start;     // First sector for file data
} fs_header_t;

// Save filesystem to storage device
int filesystem_save_to_storage(storage_device_t* device) {
    if (!device) {
        vga_puts("Error: No storage device specified\n");
        return -1;
    }
    
    vga_puts("Saving filesystem to ");
    vga_puts(device->name);
    vga_puts("...\n");
    
    // Create filesystem header
    fs_header_t header;
    memory_copy(header.magic, "PINEFS\0\0", 8);
    header.version = 1;
    header.total_entries = fs.next_entry;
    header.data_start = 2; // Start file data at sector 2
    
    // Write header to sector 0
    if (device->write_sector(device, 0, &header) != 0) {
        vga_puts("Error: Failed to write filesystem header\n");
        return -1;
    }
    
    // Write file entries to sector 1
    if (device->write_sector(device, 1, fs.entries) != 0) {
        vga_puts("Error: Failed to write file entries\n");
        return -1;
    }
    
    // Write file data starting from sector 2
    uint32_t current_sector = header.data_start;
    for (int i = 0; i < fs.next_entry; i++) {
        if (fs.entries[i].used && fs.entries[i].type == FILE_TYPE_FILE && fs.entries[i].data) {
            // Calculate sectors needed for this file
            uint32_t sectors_needed = (fs.entries[i].size + device->sector_size - 1) / device->sector_size;
            
            // Write file data
            for (uint32_t s = 0; s < sectors_needed; s++) {
                uint8_t sector_data[512] = {0}; // Clear sector
                uint32_t bytes_to_copy = device->sector_size;
                uint32_t offset = s * device->sector_size;
                
                if (offset + bytes_to_copy > fs.entries[i].size) {
                    bytes_to_copy = fs.entries[i].size - offset;
                }
                
                if (bytes_to_copy > 0) {
                    memory_copy(sector_data, fs.entries[i].data + offset, bytes_to_copy);
                }
                
                if (device->write_sector(device, current_sector + s, sector_data) != 0) {
                    vga_puts("Error: Failed to write file data\n");
                    return -1;
                }
            }
            
            current_sector += sectors_needed;
        }
    }
    
    vga_puts("Filesystem saved successfully\n");
    return 0;
}

// Load filesystem from storage device
int filesystem_load_from_storage(storage_device_t* device) {
    if (!device) {
        vga_puts("Error: No storage device specified\n");
        return -1;
    }
    
    vga_puts("Loading filesystem from ");
    vga_puts(device->name);
    vga_puts("...\n");
    
    // Read filesystem header with error checking
    fs_header_t header;
    memory_set(&header, 0, sizeof(header)); // Initialize header
    
    if (device->read_sector(device, 0, &header) != 0) {
        vga_puts("Error: Failed to read filesystem header\n");
        return -1;
    }
    
    // Verify magic number
    if (memory_compare(header.magic, "PINEFS\0\0", 8) != 0) {
        vga_puts("Error: Invalid filesystem format\n");
        return -1;
    }
    
    vga_puts("Valid filesystem found, loading...\n");
    
    // Validate entry count to prevent buffer overflow
    if (header.total_entries > MAX_FILES + MAX_DIRS || header.total_entries == 0) {
        vga_puts("Error: Invalid entry count in saved filesystem\n");
        return -1;
    }
    
    // Clear existing filesystem entries first
    memory_set(fs.entries, 0, sizeof(fs.entries));
    
    // Read saved entries with bounds checking
    if (device->read_sector(device, 1, fs.entries) != 0) {
        vga_puts("Error: Failed to read file entries\n");
        return -1;
    }
    
    // Set the entry count
    fs.next_entry = header.total_entries;
    
    // Clear all pointers and validate entries
    for (int i = 0; i < fs.next_entry; i++) {
        fs.entries[i].parent = 0;
        fs.entries[i].children = 0;
        fs.entries[i].next = 0;
        fs.entries[i].data = 0;
        
        // Validate entry data to prevent crashes
        if (fs.entries[i].used) {
            // Ensure name is null-terminated
            fs.entries[i].name[MAX_FILENAME - 1] = '\0';
            
            // Validate file size
            if (fs.entries[i].size > MAX_FILE_SIZE) {
                fs.entries[i].size = MAX_FILE_SIZE;
            }
            
            // Validate file type
            if (fs.entries[i].type != FILE_TYPE_FILE && fs.entries[i].type != FILE_TYPE_DIR) {
                fs.entries[i].used = 0; // Mark as unused if invalid
            }
        }
    }
    
    // Find root directory
    fs.root = 0;
    for (int i = 0; i < fs.next_entry; i++) {
        if (fs.entries[i].used && fs.entries[i].type == FILE_TYPE_DIR && 
            strcmp(fs.entries[i].name, "/") == 0) {
            fs.root = &fs.entries[i];
            break;
        }
    }
    
    if (!fs.root) {
        vga_puts("Warning: No root directory found, reinitializing...\n");
        filesystem_init();
        return 0;
    }
    
    fs.current_dir = fs.root;
    
    // Rebuild directory structure safely - first pass: clear all pointers
    for (int i = 0; i < fs.next_entry; i++) {
        if (fs.entries[i].used) {
            fs.entries[i].parent = 0;
            fs.entries[i].children = 0;
            fs.entries[i].next = 0;
        }
    }
    
    // Second pass: rebuild structure by putting all entries as children of root
    // This is a simplified approach that flattens the directory structure
    file_entry_t* last_child = 0;
    for (int i = 0; i < fs.next_entry; i++) {
        if (fs.entries[i].used && &fs.entries[i] != fs.root) {
            fs.entries[i].parent = fs.root;
            
            if (!fs.root->children) {
                fs.root->children = &fs.entries[i];
                last_child = &fs.entries[i];
            } else if (last_child) {
                last_child->next = &fs.entries[i];
                last_child = &fs.entries[i];
            }
        }
    }
    
    // Load file data with proper error handling
    uint32_t current_sector = header.data_start;
    for (int i = 0; i < fs.next_entry; i++) {
        if (fs.entries[i].used && fs.entries[i].type == FILE_TYPE_FILE && fs.entries[i].size > 0) {
            // Validate file size before allocation
            if (fs.entries[i].size > MAX_FILE_SIZE) {
                vga_puts("Warning: File too large, truncating\n");
                fs.entries[i].size = MAX_FILE_SIZE;
            }
            
            // Allocate memory for file data with null check
            fs.entries[i].data = memory_alloc(MAX_FILE_SIZE);
            if (!fs.entries[i].data) {
                vga_puts("Warning: Failed to allocate memory for file: ");
                vga_puts(fs.entries[i].name);
                vga_puts("\n");
                fs.entries[i].size = 0; // Mark as empty if can't allocate
                continue;
            }
            
            // Initialize allocated memory to prevent garbage data
            memory_set(fs.entries[i].data, 0, MAX_FILE_SIZE);
            
            // Calculate sectors needed with strict bounds checking
            uint32_t sectors_needed = (fs.entries[i].size + device->sector_size - 1) / device->sector_size;
            if (sectors_needed > 8) { // Limit to 8 sectors (4KB max)
                vga_puts("Warning: File too large, limiting to 8 sectors\n");
                sectors_needed = 8;
                fs.entries[i].size = 8 * device->sector_size;
                if (fs.entries[i].size > MAX_FILE_SIZE) {
                    fs.entries[i].size = MAX_FILE_SIZE;
                }
            }
            
            // Validate current_sector to prevent reading beyond device
            if (current_sector + sectors_needed > device->total_sectors) {
                vga_puts("Warning: File data beyond device capacity, skipping\n");
                memory_free(fs.entries[i].data);
                fs.entries[i].data = 0;
                fs.entries[i].size = 0;
                continue;
            }
            
            // Read file data with comprehensive error checking
            int read_success = 1;
            for (uint32_t s = 0; s < sectors_needed && read_success; s++) {
                uint8_t sector_data[512];
                memory_set(sector_data, 0, sizeof(sector_data)); // Initialize sector buffer
                
                if (device->read_sector(device, current_sector + s, sector_data) == 0) {
                    uint32_t bytes_to_copy = device->sector_size;
                    uint32_t offset = s * device->sector_size;
                    
                    // Multiple bounds checks to prevent buffer overflow
                    if (offset >= MAX_FILE_SIZE) {
                        vga_puts("Warning: Offset exceeds file buffer\n");
                        break;
                    }
                    if (offset >= fs.entries[i].size) {
                        break; // Don't read beyond file size
                    }
                    if (offset + bytes_to_copy > fs.entries[i].size) {
                        bytes_to_copy = fs.entries[i].size - offset;
                    }
                    if (offset + bytes_to_copy > MAX_FILE_SIZE) {
                        bytes_to_copy = MAX_FILE_SIZE - offset;
                    }
                    
                    if (bytes_to_copy > 0 && fs.entries[i].data) {
                        memory_copy(fs.entries[i].data + offset, sector_data, bytes_to_copy);
                    }
                } else {
                    vga_puts("Warning: Failed to read sector ");
                    // Simple sector number display
                    uint32_t sector_num = current_sector + s;
                    char sector_str[16];
                    int j = 0;
                    do {
                        sector_str[j++] = '0' + (sector_num % 10);
                        sector_num /= 10;
                    } while (sector_num > 0);
                    for (int k = j - 1; k >= 0; k--) {
                        vga_putchar(sector_str[k]);
                    }
                    vga_puts("\n");
                    read_success = 0;
                }
            }
            
            // If read failed, clean up and mark file as empty
            if (!read_success) {
                if (fs.entries[i].data) {
                    memory_free(fs.entries[i].data);
                    fs.entries[i].data = 0;
                }
                fs.entries[i].size = 0;
            }
            
            current_sector += sectors_needed;
        }
    }
    
    vga_puts("Filesystem loaded successfully\n");
    return 0;
}

// Format storage device with empty filesystem
int filesystem_format_storage(storage_device_t* device) {
    if (!device) {
        vga_puts("Error: No storage device specified\n");
        return -1;
    }
    
    vga_puts("Formatting ");
    vga_puts(device->name);
    vga_puts("...\n");
    
    // Create empty filesystem header
    fs_header_t header;
    memory_copy(header.magic, "PINEFS\0\0", 8);
    header.version = 1;
    header.total_entries = 0;
    header.data_start = 2;
    
    // Write header
    if (device->write_sector(device, 0, &header) != 0) {
        vga_puts("Error: Failed to write filesystem header\n");
        return -1;
    }
    
    // Clear file entries sector
    uint8_t empty_sector[512] = {0};
    if (device->write_sector(device, 1, empty_sector) != 0) {
        vga_puts("Error: Failed to clear file entries\n");
        return -1;
    }
    
    vga_puts("Storage device formatted successfully\n");
    return 0;
}
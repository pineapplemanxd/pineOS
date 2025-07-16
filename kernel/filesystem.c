#include "filesystem.h"
#include "io.h"
#include "string.h"

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
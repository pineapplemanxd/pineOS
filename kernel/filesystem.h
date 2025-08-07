#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "memory.h"
#include "storage.h"

// File system constants
#define MAX_FILENAME 32
#define MAX_PATH 256
#define MAX_FILES 100
#define MAX_DIRS 50
#define MAX_FILE_SIZE 4096

// File types
#define FILE_TYPE_FILE 1
#define FILE_TYPE_DIR  2

// File entry structure
typedef struct file_entry {
    char name[MAX_FILENAME];
    int type;                    // FILE_TYPE_FILE or FILE_TYPE_DIR
    int size;                    // File size in bytes
    char* data;                  // File content (for files)
    struct file_entry* parent;   // Parent directory
    struct file_entry* children; // First child (for directories)
    struct file_entry* next;     // Next sibling
    int used;                    // 1 if entry is used, 0 if free
} file_entry_t;

// File system state
typedef struct filesystem {
    file_entry_t* root;
    file_entry_t* current_dir;
    file_entry_t entries[MAX_FILES + MAX_DIRS];
    int next_entry;
} filesystem_t;

// File system functions
void filesystem_init(void);
file_entry_t* filesystem_create_file(const char* name, int type);
file_entry_t* filesystem_find_file(const char* path);
int filesystem_mkdir(const char* name);
int filesystem_touch(const char* name);
int filesystem_write_file(const char* name, const char* content);
char* filesystem_read_file(const char* name);
int filesystem_ls(const char* path);
int filesystem_cd(const char* path);
int filesystem_pwd(void);
int filesystem_rm(const char* name);
int filesystem_rmdir(const char* name);
int filesystem_cp(const char* src, const char* dest);
void filesystem_tree(const char* path, int depth);

// Path utilities
char* filesystem_get_absolute_path(const char* relative_path);
char* filesystem_get_parent_path(const char* path);
char* filesystem_get_filename(const char* path);
int filesystem_path_exists(const char* path);

// Persistent storage functions
int filesystem_save_to_storage(storage_device_t* device);
int filesystem_load_from_storage(storage_device_t* device);
int filesystem_format_storage(storage_device_t* device);

// Global filesystem instance
extern filesystem_t fs;

#endif 
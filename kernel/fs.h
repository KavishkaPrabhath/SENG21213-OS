#ifndef FS_H
#define FS_H

#include "../include/types.h"

/*
 * Stage 4 - Simple RAM File System
 *
 * A small in-memory file system used to demonstrate
 * basic file and directory operations.
 */

#define FS_MAX_FILES       16
#define FS_MAX_NAME        32
#define FS_MAX_FILE_SIZE   512

typedef struct {
    char name[FS_MAX_NAME];
    char data[FS_MAX_FILE_SIZE];
    uint32_t size;
    int used;
} fs_file_t;

/* Initialize the file system */
void fs_init(void);

/* Create a new empty file */
int fs_create(const char *name);

/* Write data to a file */
int fs_write(const char *name, const char *data);

/* Find a file by name */
fs_file_t *fs_find(const char *name);

/* Delete a file */
int fs_delete(const char *name);

/* File-system statistics */
uint32_t fs_file_count(void);

/* Access file table for shell commands */
fs_file_t *fs_get_file(uint32_t index);

#endif

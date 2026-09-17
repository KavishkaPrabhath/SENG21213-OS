#include "fs.h"

/*
 * Stage 4 - Simple RAM File System
 *
 * Files are stored entirely in memory. This provides the
 * basic create, write, find and delete operations needed
 * by the kernel shell.
 */

static fs_file_t files[FS_MAX_FILES];
static uint32_t file_count = 0;

/* Simple string comparison */
static int fs_strcmp(const char *a, const char *b)
{
    while (*a && (*a == *b)) {
        a++;
        b++;
    }

    return (uint8_t)*a - (uint8_t)*b;
}

/* Copy a string with a maximum length */
static void fs_strcpy(char *dest, const char *src, uint32_t max)
{
    uint32_t i = 0;

    if (max == 0)
        return;

    while (src[i] && i < max - 1) {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';
}

/* Initialize file system */
void fs_init(void)
{
    uint32_t i;

    file_count = 0;

    for (i = 0; i < FS_MAX_FILES; i++) {
        files[i].used = 0;
        files[i].size = 0;
        files[i].name[0] = '\0';
        files[i].data[0] = '\0';
    }
}

/* Find a file by name */
fs_file_t *fs_find(const char *name)
{
    uint32_t i;

    for (i = 0; i < FS_MAX_FILES; i++) {
        if (files[i].used && fs_strcmp(files[i].name, name) == 0) {
            return &files[i];
        }
    }

    return 0;
}

/* Create an empty file */
int fs_create(const char *name)
{
    uint32_t i;

    if (!name || !name[0])
        return -1;

    if (fs_find(name))
        return -1;

    for (i = 0; i < FS_MAX_FILES; i++) {
        if (!files[i].used) {
            files[i].used = 1;
            files[i].size = 0;

            fs_strcpy(files[i].name, name, FS_MAX_NAME);
            files[i].data[0] = '\0';

            file_count++;
            return 0;
        }
    }

    return -1;
}

/* Write text into a file */
int fs_write(const char *name, const char *data)
{
    fs_file_t *file;
    uint32_t i = 0;

    if (!data)
        return -1;

    file = fs_find(name);

    if (!file)
        return -1;

    while (data[i] && i < FS_MAX_FILE_SIZE - 1) {
        file->data[i] = data[i];
        i++;
    }

    file->data[i] = '\0';
    file->size = i;

    return (int)i;
}

/* Delete a file */
int fs_delete(const char *name)
{
    fs_file_t *file = fs_find(name);

    if (!file)
        return -1;

    file->used = 0;
    file->size = 0;
    file->name[0] = '\0';
    file->data[0] = '\0';

    if (file_count > 0)
        file_count--;

    return 0;
}

/* Number of files currently stored */
uint32_t fs_file_count(void)
{
    return file_count;
}

/* Return entry from file table */
fs_file_t *fs_get_file(uint32_t index)
{
    if (index >= FS_MAX_FILES)
        return 0;

    return &files[index];
}


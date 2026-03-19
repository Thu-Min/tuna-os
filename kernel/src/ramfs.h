#pragma once

#include <stdint.h>

#define RAMFS_NAME_MAX 64

struct ramfs_file {
    char              name[RAMFS_NAME_MAX];
    void             *data;
    uint64_t          size;
    struct ramfs_file *next;
};

void                     ramfs_init(void);
struct ramfs_file        *ramfs_create(const char *name, const void *data, uint64_t size);
const struct ramfs_file  *ramfs_find(const char *name);
void                     ramfs_list(void);

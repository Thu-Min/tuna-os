#include "ramfs.h"
#include "kheap.h"
#include "serial.h"

#include <stddef.h>

static struct ramfs_file *file_list;

static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

void ramfs_init(void) {
    file_list = NULL;
    serial_write("[ramfs] initialized\n");
}

struct ramfs_file *ramfs_create(const char *name, const void *data, uint64_t size) {
    struct ramfs_file *f = kmalloc(sizeof(struct ramfs_file));
    if (!f) {
        serial_write("[ramfs] failed to allocate file node\n");
        return NULL;
    }

    /* Copy name, truncate to RAMFS_NAME_MAX - 1 */
    uint32_t i = 0;
    while (name[i] && i < RAMFS_NAME_MAX - 1) {
        f->name[i] = name[i];
        i++;
    }
    f->name[i] = '\0';

    /* Allocate and copy data */
    f->data = kmalloc(size);
    if (!f->data) {
        serial_write("[ramfs] failed to allocate file data\n");
        kfree(f);
        return NULL;
    }

    const uint8_t *src = (const uint8_t *)data;
    uint8_t *dst = (uint8_t *)f->data;
    for (uint64_t j = 0; j < size; j++)
        dst[j] = src[j];

    f->size = size;

    /* Append to list */
    f->next = file_list;
    file_list = f;

    serial_write("[ramfs] created '");
    serial_write(f->name);
    serial_write("' size=");
    serial_write_dec_u64(size);
    serial_write("\n");

    return f;
}

const struct ramfs_file *ramfs_find(const char *name) {
    struct ramfs_file *f = file_list;
    while (f) {
        if (str_eq(f->name, name))
            return f;
        f = f->next;
    }
    return NULL;
}

void ramfs_list(void) {
    serial_write("[ramfs] listing files:\n");
    struct ramfs_file *f = file_list;
    uint32_t count = 0;
    while (f) {
        serial_write("  ");
        serial_write(f->name);
        serial_write("  ");
        serial_write_dec_u64(f->size);
        serial_write(" bytes\n");
        count++;
        f = f->next;
    }
    serial_write("[ramfs] total: ");
    serial_write_dec_u64(count);
    serial_write(" files\n");
}

#pragma once

#include <assert.h>
#include <errno.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * 1024 inodes (1-based)
 * Block bitmap
 * 512 blocks
 */

int check_error(const char* msg) {
    if (errno != 0) {
        perror(msg);
    }
    return 0;
}

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define ASSERTED && check_error(STRINGIZE(__LINE__))



#define INODES_COUNT 1024
#define MAX_BLOCKS_PER_INODE 16
#define FILE_NAME_LEN 64
#define BLOCK_SIZE 512

enum InodeType {
    REGULAR, DIRECTORY
};

typedef struct Inode {
    uint32_t size;
    uint32_t hard_links;
    enum InodeType type;
    uint32_t blocks[MAX_BLOCKS_PER_INODE];
}__attribute__ ((packed)) inode_t;

typedef struct BlockBitmap {
    uint8_t bitmap[MAX_BLOCKS_PER_INODE * INODES_COUNT / sizeof(uint8_t)];
}__attribute__ ((packed)) block_bitmap_t;

typedef struct DirEntry {
    uint32_t inode;
    char name[FILE_NAME_LEN];
}__attribute__ ((packed)) dir_entry_t;

typedef struct filesystem {
    FILE *disk_device;
    block_bitmap_t block_bitmap;
} filesystem_t;


int is_inode_empty(inode_t * inode) {
    return inode->hard_links == 0;
}

uint32_t disk_offset_inode(uint32_t inode_idx) {
    return (inode_idx - 1) * sizeof(inode_t);
}
uint32_t disk_offset_bitmap() {
    return INODES_COUNT * sizeof(inode_t);
}
uint32_t disk_offset_block(uint32_t block_idx) {
    return INODES_COUNT * sizeof(inode_t) + sizeof(block_bitmap_t) + block_idx * BLOCK_SIZE;
}

void read_inode(filesystem_t * fs, uint32_t idx, inode_t* inode) {
    fseek(fs->disk_device, disk_offset_inode(idx), SEEK_SET) ASSERTED;
    fread(inode, sizeof(inode_t), 1, fs->disk_device) ASSERTED;
}
void write_inode(filesystem_t * fs, uint32_t idx, inode_t* inode) {
    fseek(fs->disk_device, disk_offset_inode(idx), SEEK_SET) ASSERTED;
    fwrite(inode, sizeof(inode_t), 1, fs->disk_device) ASSERTED;
}
void write_bitmap(filesystem_t * fs) {
    fseek(fs->disk_device, disk_offset_bitmap(), SEEK_SET) ASSERTED;
    fwrite(&fs->block_bitmap, sizeof(block_bitmap_t), 1, fs->disk_device) ASSERTED;
}
void read_block(filesystem_t * fs, uint32_t idx, void* block) {
    fseek(fs->disk_device, disk_offset_block(idx), SEEK_SET) ASSERTED;
    fread(block, BLOCK_SIZE, 1, fs->disk_device) ASSERTED;
}
void write_block(filesystem_t * fs, uint32_t idx, const void* block, uint32_t size) {
    assert(size <= BLOCK_SIZE);
    fseek(fs->disk_device, disk_offset_block(idx), SEEK_SET) ASSERTED;
    fwrite(block, size, 1, fs->disk_device) ASSERTED;
}

uint32_t fs_find_empty_inode(filesystem_t * fs) {
    for (uint32_t i = 1; i < INODES_COUNT; ++i) {
        inode_t inode;
        read_inode(fs, i, &inode);
        if (is_inode_empty(&inode)) {
            printf("Allocated inode %d\n", i);
            return i;
        }
    }

    printf("FATAL: no more inodes\n");
    return -1;
}

uint32_t fs_alloc_block(filesystem_t * fs) {
    block_bitmap_t * bb = &fs->block_bitmap;
    for (uint32_t i = 0; i < sizeof(bb->bitmap) / sizeof(uint8_t); ++i) {
        uint8_t value = bb->bitmap[i];
        if (value != 0xFF) {
            for (uint32_t bit = 0; bit < 8; ++bit) {
                if ((value & (1 << bit)) == 0) {
                    bb->bitmap[i] |= (1 << bit);
                    write_bitmap(fs);
                    uint32_t block_idx = i * 8 + bit;

                    printf("Allocated block %d\n", block_idx);
                    return block_idx;
                }
            }
        }
    }

    printf("FATAL: no more blocks\n");
    return -1;
}

void fs_dealloc_block(filesystem_t * fs, uint32_t idx) {
    block_bitmap_t * bb = &fs->block_bitmap;
    bb->bitmap[idx / 8] &= ~(1 << (idx % 8));
    write_bitmap(fs);
    printf("Deallocated block %d\n", idx);
}

void fs_init_dir_block(filesystem_t * fs, uint32_t inode_idx, uint32_t parent_inode) {
    // Note: function does NOT link child-dir to its parent

    dir_entry_t entries[2] = {
            {
                    .inode = inode_idx,
                    .name = "."
            },
            {
                    .inode = parent_inode,
                    .name = ".."
            }
    };

    uint32_t hard_links = (inode_idx == parent_inode) ? 2 : 1;
    inode_t inode = {
            .size = sizeof(entries),
            .hard_links = hard_links,
            .type = DIRECTORY,
            .blocks = {fs_alloc_block(fs)}
    };
    write_inode(fs, inode_idx, &inode);
    write_block(fs, inode.blocks[0], entries, sizeof(entries));
}

void fs_init(filesystem_t * fs, const char* path) {
    fs->disk_device = fopen(path, "r+");
    if (!fs->disk_device) {
        check_error("fs_init: fopen");
        return;
    }

    fseek(fs->disk_device, disk_offset_bitmap(), SEEK_SET) ASSERTED;
    fread(&fs->block_bitmap, sizeof(block_bitmap_t), 1, fs->disk_device) ASSERTED;
}

void fs_create(filesystem_t * fs, const char* path) {
    fs->disk_device = fopen(path, "w+");

    // Allocate inodes
    char zero = '\0';
    fseek(fs->disk_device, INODES_COUNT * sizeof(inode_t), SEEK_SET) ASSERTED;
    fwrite(&zero, sizeof(zero), 1, fs->disk_device) ASSERTED;

    // Create root
    fs_init_dir_block(fs, 1, 1);
}

void fs_close(filesystem_t * fs) {
    fclose(fs->disk_device);
}

dir_entry_t * fs_listdir(filesystem_t * fs, uint32_t dir_inode_idx) {
    inode_t dir_inode;
    read_inode(fs, dir_inode_idx, &dir_inode);
    if (dir_inode.type != DIRECTORY) {
        return NULL;
    }

    uint32_t entries_cnt = dir_inode.size / sizeof(dir_entry_t);
    dir_entry_t * result = calloc(sizeof(dir_entry_t), entries_cnt + 1);

    char block_data[BLOCK_SIZE];
    read_block(fs, dir_inode.blocks[0], block_data);

    for (uint32_t read_size = 0, i = 0; read_size < dir_inode.size; read_size += sizeof(dir_entry_t), ++i) {
        dir_entry_t * dirEntry = (dir_entry_t *) (block_data + read_size);
        memcpy(result + i, dirEntry, sizeof(dir_entry_t));
    }

    return result;
}

void fs_listdir_free(dir_entry_t * result) {
    free(result);
}

void fs_link(filesystem_t * fs, uint32_t linking_inode, const char* name, uint32_t dir_inode_idx) {
    inode_t dir_inode;
    read_inode(fs, dir_inode_idx, &dir_inode);

    dir_entry_t entry = { .inode = linking_inode };
    strcpy(entry.name, name);
    char block[BLOCK_SIZE];
    read_block(fs, dir_inode.blocks[0], block);
    memcpy(block + dir_inode.size, &entry, sizeof(entry));
    write_block(fs, dir_inode.blocks[0], block, BLOCK_SIZE);

    dir_inode.size += sizeof(dir_entry_t);
    write_inode(fs, dir_inode_idx, &dir_inode);

    inode_t file_inode;
    read_inode(fs, linking_inode, &file_inode);
    ++file_inode.hard_links;
    write_inode(fs, linking_inode, &file_inode);
}

uint32_t fs_create_regular_file(filesystem_t * fs, uint32_t size, const void* data) {
    assert(size <= BLOCK_SIZE);
    uint32_t block_idx = fs_alloc_block(fs);
    write_block(fs, block_idx, data, size);

    uint32_t inode_idx = fs_find_empty_inode(fs);
    inode_t inode = {
            .size = size,
            .hard_links = 0,
            .type = REGULAR,
            .blocks = { block_idx }
    };
    write_inode(fs, inode_idx, &inode);

    return inode_idx;
}

uint32_t fs_create_directory(filesystem_t * fs, uint32_t parent_inode, const char* name) {
    uint32_t inode_idx = fs_find_empty_inode(fs);
    fs_init_dir_block(fs, inode_idx, parent_inode);
    fs_link(fs, inode_idx, name, parent_inode);
    return inode_idx;
}

uint32_t fs_parse_path(filesystem_t * fs, const char* path) {
    /* '/'
     * '/foo'
     * '/bar/'
     * '/bar/foo'
     */
    assert(path[0] == '/');
    ++path;

    uint32_t inode_idx = 1;
    inode_t inode;

    while (*path != 0) {
        char * next_slash = strchr(path, '/');  // next_slash may be end of str
        int last_entry_in_path = 0;
        if (next_slash == NULL) {
            next_slash = strchr(path, 0);
            last_entry_in_path = 1;
        }
        *next_slash = 0;

        dir_entry_t * files = fs_listdir(fs, inode_idx);
        if (files == NULL) {
            printf("parse_path: incorrect path, inode %d\n", inode_idx);
            return 0;
        }
        dir_entry_t * cur_file = files;

        uint32_t next_inode = 0;
        while (cur_file->inode != 0) {
            if (strcmp(path, cur_file->name) == 0) {
                next_inode = cur_file->inode;
                break;
            }
            ++cur_file;
        }

        fs_listdir_free(files);

        if (next_inode == 0) {
            printf("parse_path: not found %s in inode %d\n", path, inode_idx);
            return 0;
        }
        inode_idx = next_inode;

        path = next_slash;
        if (!last_entry_in_path) {
            ++path;
        }
    }

    return inode_idx;
}

uint32_t fs_read_regular_file(filesystem_t * fs, uint32_t inode_idx, void* buffer) {
    inode_t inode;
    read_inode(fs, inode_idx, &inode);
    if (inode.size == 0) {
        return 0;
    }

    read_block(fs, inode.blocks[0], buffer);
    return inode.size;
}

void fs_unlink(filesystem_t * fs, const char* name, uint32_t dir_inode_idx);

void fs_decrement_links(filesystem_t * fs, uint32_t inode_idx) {
    inode_t inode;
    read_inode(fs, inode_idx, &inode);

    if (inode.hard_links == 0) {
        return;
    } else {
        --inode.hard_links;
    }

    if (inode.hard_links == 0 || (inode.hard_links == 1 && inode.type == DIRECTORY)) {
        assert(inode.type != DIRECTORY || inode.size == 2 * sizeof(dir_entry_t));

        fs_dealloc_block(fs, inode.blocks[0]);  // deallocate block
        memset(&inode, 0, sizeof(inode_t));
        write_inode(fs, inode_idx, &inode);  // deallocate inode
    }
}

void fs_unlink(filesystem_t * fs, const char* name, uint32_t dir_inode_idx) {
    inode_t inode;
    read_inode(fs, dir_inode_idx, &inode);
    char block[BLOCK_SIZE];
    read_block(fs, inode.blocks[0], block);
    dir_entry_t * list = (dir_entry_t *) block;

    uint32_t unlinked_inode = 0;
    uint32_t i = 0;
    for (; list->inode != 0; ++i, ++list) {
        if (strcmp(list->name, name) == 0) {
            unlinked_inode = list->inode;
            break;
        }
    }

    inode_t inode_of_child;
    read_inode(fs, unlinked_inode, &inode_of_child);
    if (inode_of_child.type == DIRECTORY && inode_of_child.size > 2 * sizeof(dir_entry_t)) {
        printf("fs_unlink: cannot unlink dir with files in it\n");
        return;
    }

    // Remove from dir
    memmove(list, list + 1, inode.size - (i + 1) * sizeof(dir_entry_t));
    memset(block + (inode.size - sizeof(dir_entry_t)), 0, sizeof(dir_entry_t));
    inode.size -= sizeof(dir_entry_t);

    if (unlinked_inode == 0) {
        printf("fs_unlink: in dir %d not found %s\n", dir_inode_idx, name);
        return;
    }

    write_block(fs, inode.blocks[0], block, BLOCK_SIZE);
    write_inode(fs, dir_inode_idx, &inode);

    fs_decrement_links(fs, unlinked_inode);
}

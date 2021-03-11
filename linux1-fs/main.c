#include <stdio.h>
#include <string.h>
#include "fs.h"

void print_help() {
    printf("Usage: <path_to_filesystem> <operation>\n"
           "Supported operations: create ls link write cat mkdir unlink\n");
}

int main(int argc, char** argv) {
    if (argc < 3) {
        print_help();
        return 0;
    }

    char* filepath = argv[1];
    filesystem_t fs = {0};

    if (strcmp(argv[2], "create") == 0) {
        fs_create(&fs, filepath);
    } else {
        fs_init(&fs, filepath);
        if (strcmp(argv[2], "ls") == 0) {
            const char *dir_path = "/";
            if (argc > 3) {
                dir_path = argv[3];
            }
            uint32_t dir_inode = fs_parse_path(&fs, dir_path);

            if (dir_inode != 0) {
                printf("TYPE\tSIZE\tLINKS\tINODE\tNAME\n");

                dir_entry_t *result = fs_listdir(&fs, dir_inode);
                dir_entry_t *iter = result;
                while (iter->inode != 0) {
                    inode_t inode = {0};
                    read_inode(&fs, iter->inode, &inode);

                    char type = inode.type == REGULAR ? 'R' : 'D';
                    printf("%c\t%d\t%d\t%d\t%s\n", type, inode.size, inode.hard_links, iter->inode, iter->name);
                    ++iter;
                }
                fs_listdir_free(result);
            }
        } else if (strcmp(argv[2], "link") == 0) {
            if (argc < 6) {
                printf("need args: <path_to_linking> <link_dir> <link_name>\n");
            } else {
                const char *path_to_linking = argv[3];
                const char *link_dir = argv[4];
                const char *link_name = argv[5];
                uint32_t linking_inode = fs_parse_path(&fs, path_to_linking);
                uint32_t dir_inode = fs_parse_path(&fs, link_dir);
                fs_link(&fs, linking_inode, link_name, dir_inode);
            }
        } else if (strcmp(argv[2], "write") == 0) {
            if (argc < 6) {
                printf("need args: <dirpath> <name> <text>\n");
            } else {
                const char *dir_path = argv[3];
                const char *name = argv[4];
                const char *data = argv[5];
                uint32_t len = strlen(data);
                uint32_t dir_inode = fs_parse_path(&fs, dir_path);
                uint32_t inode_idx = fs_create_regular_file(&fs, len, data);
                fs_link(&fs, inode_idx, name, dir_inode);
            }
        } else if (strcmp(argv[2], "cat") == 0) {
            if (argc < 4) {
                printf("need args: <filepath>\n");
            } else {
                const char *path = argv[3];
                uint32_t inode = fs_parse_path(&fs, path);

                char block[BLOCK_SIZE];
                uint32_t size = fs_read_regular_file(&fs, inode, block);
                block[size] = 0;
                printf("%s\n", block);
            }
        } else if (strcmp(argv[2], "mkdir") == 0) {
            if (argc < 5) {
                printf("need args: <parent_dir_path> <dir_name>\n");
            } else {
                const char *dir_path = argv[3];
                const char *name = argv[4];
                uint32_t dir_inode = fs_parse_path(&fs, dir_path);
                fs_create_directory(&fs, dir_inode, name);
            }
        } else if (strcmp(argv[2], "unlink") == 0) {
            if (argc < 5) {
                printf("need args: <dir_path> <name>\n");
            } else {
                const char *path = argv[3];
                const char *name = argv[4];
                uint32_t inode = fs_parse_path(&fs, path);
                fs_unlink(&fs, name, inode);
            }
        } else {
            print_help();
        }
    }

    fs_close(&fs);
    return 0;
}

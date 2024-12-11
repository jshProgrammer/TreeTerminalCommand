//
// Created by Joshua Pfennig on 11.12.24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_INDENT 128

// Global options0
int option_show_full_path = 0;
int option_max_depth = -1;
int option_follow_symlinks = 0;
int option_show_hidden = 0;
int option_show_file_sizes = 0;

void print_usage(const char *program_name) {
    printf("Usage: %s [options] [directory]\n", program_name);
    printf("Options:\n");
    printf("  -f            Show full path\n");
    printf("  -L <n>        Limit the depth of the tree to <n>\n");
    printf("  -l            Follow symbolic links\n");
    printf("  -a            Show hidden files\n");
    printf("  -s            Show file sizes\n");
    printf("  -h            Show this help message\n");
}

void print_indentation(int level) {
    for (int i = 0; i < level; ++i) {
        printf("|   ");
    }
}

void print_file_info(const char *path, const struct stat *statbuf) {
    if (option_show_file_sizes) {
        printf(" [%ld bytes]", statbuf->st_size);
    }
    printf("\n");
}

void print_tree(const char *path, int level) {
    if (option_max_depth != -1 && level > option_max_depth) {
        return;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Cannot open directory '%s': %s\n", path, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (!option_show_hidden && entry->d_name[0] == '.') {
            continue;
        }

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if ((option_follow_symlinks ? stat : lstat)(full_path, &statbuf) == -1) {
            fprintf(stderr, "Cannot stat '%s': %s\n", full_path, strerror(errno));
            continue;
        }

        print_indentation(level);
        printf("|-- %s", option_show_full_path ? full_path : entry->d_name);

        print_file_info(full_path, &statbuf);

        if (S_ISDIR(statbuf.st_mode)) {
            print_tree(full_path, level + 1);
        }
    }

    closedir(dir);
}

void print_directory_summary(const char *path) {
    int total_files = 0, total_dirs = 0;

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Cannot open directory '%s': %s\n", path, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (!option_show_hidden && entry->d_name[0] == '.') {
            continue;
        }

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if ((option_follow_symlinks ? stat : lstat)(full_path, &statbuf) == -1) {
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            total_dirs++;
        } else {
            total_files++;
        }
    }

    closedir(dir);

    printf("\nSummary:\n");
    printf("Total directories: %d\n", total_dirs);
    printf("Total files: %d\n", total_files);
}

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "fL:lasmh")) != -1) {
        switch (opt) {
            case 'f':
                option_show_full_path = 1;
                break;
            case 'L':
                option_max_depth = atoi(optarg);
                if (option_max_depth < 0) {
                    fprintf(stderr, "Invalid depth value: %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case 'l':
                option_follow_symlinks = 1;
                break;
            case 'a':
                option_show_hidden = 1;
                break;
            case 's':
                option_show_file_sizes = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    const char *start_path = (optind < argc) ? argv[optind] : ".";

    struct stat statbuf;
    if (stat(start_path, &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Invalid directory: %s\n", start_path);
        return EXIT_FAILURE;
    }

    print_tree(start_path, 0);
    print_directory_summary(start_path);

    return EXIT_SUCCESS;
}

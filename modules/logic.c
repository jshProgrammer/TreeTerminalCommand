#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "h-Files/globals.h"
#include "h-Files/queue.h"
#include "h-Files/logic.h"
#include "h-Files/tree.h"
#include "h-Files/printConsole.h"
#include "h-Files/printFile.h"
#include <getopt.h>


int compare_entries(const struct dirent **a, const struct dirent **b) {
    if (option_dirs_first) {
        int is_dir_a = (*a)->d_type == DT_DIR;
        int is_dir_b = (*b)->d_type == DT_DIR;
        if (is_dir_a != is_dir_b) {
            return is_dir_b - is_dir_a;
        }
    }
    if (option_sort_time) {
        struct stat stat_a, stat_b;
        stat((*a)->d_name, &stat_a);
        stat((*b)->d_name, &stat_b);
        return option_reverse_sort ? stat_b.st_mtime - stat_a.st_mtime : stat_a.st_mtime - stat_b.st_mtime;
    }
    return option_reverse_sort ? strcasecmp((*b)->d_name, (*a)->d_name) : strcasecmp((*a)->d_name, (*b)->d_name);
}

void increment_counters(bool is_dir) {
    pthread_mutex_lock(&counter_mutex);
    if (is_dir) {
        total_dirs++;
    } else {
        total_files++;
    }
    pthread_mutex_unlock(&counter_mutex);
}

void process_directory(const char *path, int level, TreeNode *parent) {
    for (int i = 0; i < pruned_dir_count; i++) {
        if (strcmp(path, pruned_directories[i]) == 0) {
            return;
        }
    }

    if (option_max_depth != -1 && level > option_max_depth) {
        return;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Cannot open directory '%s': %s\n", path, strerror(errno));
        return;
    }

    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
        fprintf(stderr, "Cannot stat '%s': %s\n", path, strerror(errno));
        closedir(dir);
        return;
    }


    TreeNode *node = NULL;
    if(option_output_csv || option_output_json) {
        node = create_node(path, statbuf.st_size, level, S_ISDIR(statbuf.st_mode));
        if (parent) {
            add_child(parent, node);
        } else if (level == 0) {
            pthread_mutex_lock(&tree_mutex);
            global_root = node;
            pthread_mutex_unlock(&tree_mutex);
        }
    }

    struct dirent **entries;
    int count = scandir(path, &entries, NULL, compare_entries);
    if (count < 0) {
        fprintf(stderr, "Error scanning directory '%s': %s\n", path, strerror(errno));
        closedir(dir);
        return;
    }

    if(option_file_limit != -1) {
        int file_count = 0;
        for (int i = 0; i < count; i++) {
            struct dirent *entry = entries[i];
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char full_path[max_path];
                if (snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name) < max_path) {
                    struct stat statbuf;
                    if ((option_follow_symlinks ? stat : lstat)(full_path, &statbuf) == 0) {
                        if (!S_ISDIR(statbuf.st_mode)) {
                            file_count++;
                        }
                    }
                }
            }
            if (file_count > option_file_limit) {
                printf("Skipping directory '%s' due to file limit (%d files)\n", path, file_count);
                closedir(dir);
                return;
            }
        }

    }

    for (int i = 0; i < count; i++) {
        struct dirent *entry = entries[i];
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (!option_show_hidden && entry->d_name[0] == '.') {
            continue;
        }

        char full_path[max_path];
        if (snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name) >= max_path) {
            fprintf(stderr, "Path too long: %s/%s\n", path, entry->d_name);
            continue;
        }

        struct stat statbuf;
        if ((option_follow_symlinks ? stat : stat)(full_path, &statbuf) == -1) {
            fprintf(stderr, "Cannot stat '%s': %s\n", full_path, strerror(errno));
            continue;
        }

        if (output_file != NULL) {
            output_to_txt_file(full_path);
        }

        print_indentation_in_sysout(level);
        printf("|-- %s", option_show_full_path ? full_path : entry->d_name);

        print_file_info(&statbuf);

        if (option_show_user || option_show_group) {
            printf(" [");

            if (option_show_user) {
                struct passwd *pwd = getpwuid(statbuf.st_uid);
                if (pwd) {
                    printf("User: %s", pwd->pw_name);
                } else {
                    printf("User: %d", statbuf.st_uid);
                }
            }

            if (option_show_group) {
                if (option_show_user) {
                    printf(", ");
                }
                struct group *grp = getgrgid(statbuf.st_gid);
                if (grp) {
                    printf("Group: %s", grp->gr_name);
                } else {
                    printf("Group: %d", statbuf.st_gid);
                }
            }

            printf("]");
        }

        if (S_ISDIR(statbuf.st_mode)) {
            increment_counters(true);
            process_directory(full_path, level + 1, node);
        } else {
            increment_counters(false);
            if(option_output_csv || option_output_json) {
                TreeNode *file_node = create_node(full_path, statbuf.st_size, level+1, 0);
                add_child(global_root, file_node);
            }
        }
    }

    closedir(dir);
}

void *worker(void *arg) {
    Queue *path_queue = (Queue *)arg;
    char path[max_path];
    int level;

    while (dequeue(path_queue, path, &level)) {

        process_directory(path, level, NULL);
    }

    return NULL;
}

const char *parse_options(int argc, char *argv[]) {
    int opt;

    static struct option long_options[] = {
        {"noSum", no_argument, &option_show_summary, 0},
        {"dirsfirst", no_argument, &option_dirs_first, 1},
        {"output-json", required_argument, NULL, 'j'},
        {"output-csv", required_argument, NULL, 'c'},
        {"filelimit", required_argument, NULL, 'F'},
        {"prune", no_argument, NULL, 'P'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "fL:lasdritugpPj:c:o:Fh", long_options, NULL)) != -1) {
        if (opt == 0) {
            continue;
        }
        switch (opt) {
            case 'f':
                option_show_full_path = 1;
                break;
            case 'L':
                option_max_depth = atoi(optarg);
                if (option_max_depth < 0) {
                    fprintf(stderr, "Invalid depth value: %s\n", optarg);
                    exit(EXIT_FAILURE);
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
            case 'd':
                option_dirs_only = 1;
                break;
            case 'r':
                option_reverse_sort = 1;
                break;
            case 'i':
                option_ignore_case = 1;
                break;
            case 't':
                option_sort_time = 1;
                break;
            case 'u':
                option_show_user = 1;
                break;
            case 'g':
                option_show_group = 1;
                break;
            case 'p':
            case 'P':
                if (pruned_dir_count < MAX_PRUNED_DIRS) {
                    if (optarg != NULL && strlen(optarg) > 0) {
                        char resolved_path[max_path];
                        if (realpath(optarg, resolved_path) != NULL) {
                            pruned_directories[pruned_dir_count++] = strdup(resolved_path);
                        } else {
                            fprintf(stderr, "Error resolving path: %s\n", optarg);
                        }
                    } else {
                        fprintf(stderr, "Invalid directory path for prune option\n");
                    }
                } else {
                    fprintf(stderr, "Maximum number of pruned directories reached.\n");
                }
                break;
            case 'j':
                option_output_json = 1;
                output_file = optarg;
                break;
            case 'c':
                option_output_csv = 1;
                output_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'F':
                option_file_limit = atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        return argv[optind];
    } else {
        return ".";
    }
}
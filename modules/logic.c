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
#include <pwd.h>   // Für Benutzernamen (UID zu Username)
#include <grp.h>   // Für Gruppennamen (GID zu Groupname)
#include "h-Files/globals.h"
#include "h-Files/queue.h"
#include "h-Files/logic.h"
#include "h-Files/tree.h"


int compare_entries(const struct dirent **a, const struct dirent **b) {
    if (option_dirs_first) {
        int is_dir_a = (*a)->d_type == DT_DIR;
        int is_dir_b = (*b)->d_type == DT_DIR;
        if (is_dir_a != is_dir_b) {
            return is_dir_b - is_dir_a; // Verzeichnisse zuerst
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

//TODO: use together with print_indentation method below => generalise
void print_indents_in_file(FILE *file, int indent) {
    for (int i = 0; i < indent; ++i) {
        fprintf(file, "    "); // 4 Leerzeichen pro Ebene
    }
}

void generate_json_output(FILE *file, TreeNode *node, int indent) {
    print_indents_in_file(file, indent);
    if(indent == 0) fprintf(file, "{\n");
    else {
        fprintf(file, "\n");
        print_indents_in_file(file, indent);
        fprintf(file, "{\n");
    }

    print_indents_in_file(file, indent+1);
    fprintf(file, "\"name\": \"%s\",\n", node->name);

    print_indents_in_file(file, indent+1);
    fprintf(file, "\"size\": %ld,\n", node->size);

    print_indents_in_file(file, indent+1);
    fprintf(file, "\"is_dir\": %s,\n", node->is_dir ? "true" : "false");

    print_indents_in_file(file, indent+1);
    fprintf(file, "\"children\": [\n");

    for (int i = 0; i < node->child_count; ++i) {
        generate_json_output(file, node->children[i], indent+2);
        if (i < node->child_count - 1) {
            fprintf(file, ",");
        }
    }

    if (node->child_count > 0) {
        fprintf(file, "\n"); // Neue Zeile nach der letzten Kinderausgabe
        print_indents_in_file(file, indent + 1);
    }

    print_indents_in_file(file, indent+1);
    fprintf(file, "]\n");

    print_indents_in_file(file, indent);
    fprintf(file, "}");
}

void generate_csv_output(FILE *file, TreeNode *node) {
   fprintf(file, "\"%s\",%ld,%d,%s\n", node->name, node->size, node->level, node->is_dir ? "directory" : "file");

    for (int i = 0; i < node->child_count; ++i) {
        generate_csv_output(file, node->children[i]);
    }
}

//TODO: prüfen, ob man wirklich nochmal extra alles durch iterieren muss oder ob man das in processDirectory inkludieren kann
TreeNode *build_tree(const char *path, int level) {
    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
        fprintf(stderr, "Cannot stat '%s': %s\n", path, strerror(errno));
        return NULL;
    }

    TreeNode *node = create_node(path, statbuf.st_size, level, S_ISDIR(statbuf.st_mode));

    if (node->is_dir) {
        DIR *dir = opendir(path);
        if (!dir) {
            fprintf(stderr, "Cannot open directory '%s': %s\n", path, strerror(errno));
            return node;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char full_path[max_path];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            TreeNode *child = build_tree(full_path, level + 1);
            if (child) {
                add_child(node, child);
            }
        }
        closedir(dir);
    }

    return node;
}

void output_to_file_json_csv(TreeNode *root) {
    if (!root) {
        fprintf(stderr, "Error: Tree root is NULL.\n");
        return;
    }

    FILE *file = output_file ? fopen(output_file, "w") : stdout;
    if (!file) {
        perror("Error opening output file");
        return;
    }

    if (option_output_json) {
        generate_json_output(file, root, 0);
    } else if (option_output_csv) {
        // Write CSV header
        fprintf(file, "\"Name\",\"Size\",\"Level\",\"Type\"\n");
        generate_csv_output(file, root);
    } else {
        fprintf(stderr, "No valid output format selected.\n");
    }

    if (output_file) {
        fclose(file);
    }
}



//TODO: vlt. hier noch als Parameter zstl den Dateinamen hinzufügen?
void output_to_file(const char *path, const struct stat *statbuf, int level) {

        if (output_file != NULL) {
            FILE *file = fopen(output_file, "a");
            if (!file) {
                perror("Failed to open output file");
                exit(EXIT_FAILURE);
            }
            fprintf(file, "%s\n", path);
            fclose(file);
        } else {
            printf("%s\n", path);
        }

}

void print_usage(const char *program_name) {
    printf("Usage: %s [options] [directory]\n", program_name);
    printf("Options:\n");
    printf("  -f            Show full path\n");
    printf("  -L <n>        Limit the depth of the tree to <n>\n");
    printf("  -l            Follow symbolic links\n");
    printf("  -a            Show hidden files\n");
    printf("  -s            Show file sizes\n");
    printf("  --noSum       Show no summary :)\n");
    printf("  -d            List directories only\n");
    printf("  --dirsfirst   List directories before files\n");
    printf("  -r            Sort output in reverse order\n");
    printf("  -i            Ignore case when sorting\n");
    printf("  -t            Sort by last modification time\n");
    printf("  -u            Show user name or UID if no name is available\n");
    printf("  -g            Show group name or GID if no name is available\n");
    printf("  -p <dir>      Prune (omit) specified directory from the tree\n");
    printf("  --prune <dir> Prune (omit) specified directory from the tree\n");
    printf("  --filelimit # Limit descending into directories with more than # entries\n");
    //TODO: hier vermutlich noch <file> ergänzen
    printf("  --output-json Output result in JSON format\n");
    printf("  --output-csv  Output result in CSV format\n");
    printf("  -o <file>     Send output to file\n");
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

// Thread-safe counter increment
void increment_counters(bool is_dir) {
    pthread_mutex_lock(&counter_mutex);
    if (is_dir) {
        total_dirs++;
    } else {
        total_files++;
    }
    pthread_mutex_unlock(&counter_mutex);
}

void process_directory(const char *path, int level) {
    // Check if directory should be pruned => then skip this directory
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

    // Nutze scandir für optionales Sortieren der Einträge
    struct dirent **entries;
    int count = scandir(path, &entries, NULL, compare_entries); // compare_entries nutzt option_dirs_first, option_reverse_sort usw.
    if (count < 0) {
        fprintf(stderr, "Error scanning directory '%s': %s\n", path, strerror(errno));
        closedir(dir);
        return;
    }

    if(option_file_limit != -1) {
        //TODO: gibt es eine effizientere Methode als hier nochmal mit for wirklich durchzuloopen?
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
            // Wenn  Anzahl der Dateien das Limit überschreitet => überspringe das Verzeichnis
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
            output_to_file(full_path, &statbuf, level);
        }

        print_indentation(level);
        printf("|-- %s", option_show_full_path ? full_path : entry->d_name);

        print_file_info(full_path, &statbuf);

        if (option_show_user || option_show_group) {
            printf(" [");

            if (option_show_user) {
                struct passwd *pwd = getpwuid(statbuf.st_uid);
                if (pwd) {
                    printf("User: %s", pwd->pw_name);
                } else {
                    printf("User: %d", statbuf.st_uid); // UID, falls kein Username verfügbar
                }
            }

            if (option_show_group) {
                if (option_show_user) {
                    printf(", "); // Trennzeichen, falls beide Optionen aktiv sind
                }
                struct group *grp = getgrgid(statbuf.st_gid);
                if (grp) {
                    printf("Group: %s", grp->gr_name);
                } else {
                    printf("Group: %d", statbuf.st_gid); // GID, falls kein Gruppenname verfügbar
                }
            }

            printf("]");
        }

        if (S_ISDIR(statbuf.st_mode)) {
            increment_counters(true);
            process_directory(full_path, level + 1);
        } else {
            increment_counters(false);
        }
    }

    closedir(dir);
}

void *worker(void *arg) {
    Queue *path_queue = (Queue *)arg;
    char path[max_path];
    int level;

    // Jeder Thread bearbeitet seine eigene Queue
    while (dequeue(path_queue, path, &level)) {
        process_directory(path, level);
    }

    return NULL;
}

void print_directory_summary() {
    if (option_show_summary) {
        printf("\nSummary:\n");
        printf("Total directories: %d\n", total_dirs);
        printf("Total files: %d\n", total_files);
    }
}
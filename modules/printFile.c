#include <stdio.h>
#include "h-Files/tree.h"
#include "h-Files/globals.h"
#include "h-Files/queue.h"
#include "h-Files/printFile.h"

#include <grp.h>
#include <pwd.h>

#include "h-Files/printConsole.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

void print_indents_in_file(FILE *file, int indent) {
    for (int i = 0; i < indent; ++i) {
        fprintf(file, "    ");
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
        fprintf(file, "\n");
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
void output_to_txt_file(const char *path) {
    if (output_file != NULL) {
        FILE *file = fopen(output_file, "a");
        if (!file) {
            perror("Failed to open output file");
            exit(EXIT_FAILURE);
        }

        struct stat statbuf;
        if (stat(path, &statbuf) == -1) {
            fprintf(stderr, "Cannot stat '%s': %s\n", path, strerror(errno));
            return;
        }


        if (option_show_file_sizes) {
            fprintf(file, " [%lld bytes]\n", statbuf.st_size);
        }

        if (option_show_user || option_show_group) {
            fprintf(file, " [");

            if (option_show_user) {
                struct passwd *pwd = getpwuid(statbuf.st_uid);
                if (pwd) {
                    fprintf(file, "User: %s", pwd->pw_name);
                } else {
                    fprintf(file, "User: %d", statbuf.st_uid);
                }
            }

            if (option_show_group) {
                if (option_show_user) {
                    fprintf(file, ", ");
                }
                struct group *grp = getgrgid(statbuf.st_gid);
                if (grp) {
                    fprintf(file, "Group: %s", grp->gr_name);
                } else {
                    fprintf(file, "Group: %d", statbuf.st_gid);
                }
            }

            fprintf(file, "]");
        }

        fprintf(file, "%s\n", path);
        fclose(file);
    } else {
        printf("%s\n", path);
    }
}
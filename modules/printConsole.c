#include "h-Files/globals.h"
#include "h-Files/queue.h"
#include "h-Files/logic.h"
#include "h-Files/tree.h"
#include "h-Files/printConsole.h"
#include "h-Files/printFile.h"
#include "h-Files/tests.h"

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
#include <dirent.h>


void print_usage(const char *program_name) {
    printf("Usage: %s [options] [directory]\n", program_name);
    printf("Options:\n");
    printf("  -f                    Show full path\n");
    printf("  -L <n>                Limit the depth of the tree to <n>\n");
    printf("  -l                    Follow symbolic links\n");
    printf("  -a                    Show hidden files\n");
    printf("  -s                    Show file sizes\n");
    printf("  --noSum               Show no summary :)\n");
    printf("  -d                    List directories only\n");
    printf("  --dirsfirst           List directories before files\n");
    printf("  -r                    Sort output in reverse order\n");
    printf("  -i                    Ignore case when sorting\n");
    printf("  -t                    Sort by last modification time\n");
    printf("  -u                    Show username or UID if no name is available\n");
    printf("  -g                    Show group name or GID if no name is available\n");
    printf("  -p <dir>              Prune (omit) specified directory from the tree\n");
    printf("  --prune <dir>         Prune (omit) specified directory from the tree\n");
    printf("  --filelimit #         Limit descending into directories with more than # entries\n");
    //TODO: hier vermutlich noch <file> ergänzen
    printf("  --output-json <file>  Output result in JSON format and send to file\n");
    printf("  --output-csv <file    Output result in CSV format and send to file\n");
    printf("  -o <file>             Send output to file\n");
    printf("  -h                    Show this help message\n");
}

void print_indentation_in_sysout(int level) {
    for (int i = 0; i < level; ++i) {
        printf("|   ");
    }
}

void print_file_info(const struct stat *statbuf) {
    if (option_show_file_sizes) {
        printf(" [%lld bytes]", statbuf->st_size);
    }
    printf("\n");
}

void print_directory_summary() {
    if (option_show_summary) {
        printf("\nSummary:\n");
        printf("Total directories: %d\n", total_dirs);
        printf("Total files: %d\n", total_files);
    }
}
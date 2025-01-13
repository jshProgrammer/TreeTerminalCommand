#ifndef PRINTCONSOLE_H
#define PRINTCONSOLE_H
#include "tree.h"
#include "globals.h"
#include "logic.h"
#include "queue.h"
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

void print_usage(const char *program_name);
void print_indentation_in_sysout(int level);
void print_file_info(const struct stat *statbuf);
void print_directory_summary();

#endif

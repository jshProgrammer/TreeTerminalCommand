#ifndef LOGIC_H
#define LOGIC_H
#include <stdbool.h>

int compare_entries(const struct dirent **a, const struct dirent **b);
void output_to_file(const char *path, const struct stat *statbuf, int level);
void print_usage(const char *program_name);
void print_indentation(int level);
void print_file_info(const char *path, const struct stat *statbuf);
void increment_counters(bool is_dir);
void process_directory(const char *path, int level);
void *worker(void *arg);
void print_directory_summary();


#endif

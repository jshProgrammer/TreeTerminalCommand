#ifndef LOGIC_H
#define LOGIC_H
#include <stdbool.h>

#include "tree.h"

int compare_entries(const struct dirent **a, const struct dirent **b);
void output_to_txt_file(const char *path);
void print_usage(const char *program_name);
void print_indentation_in_sysout(int level);
void print_file_info( const struct stat *statbuf);
void increment_counters(bool is_dir);
void process_directory(const char *path, int level, TreeNode *parent);
void *worker(void *arg);
void print_directory_summary();

void generate_json_output(FILE *file, TreeNode *node, int indent);
void generate_csv_output(FILE *file, TreeNode *node);
//TreeNode *build_tree(const char *path, int level);



#endif

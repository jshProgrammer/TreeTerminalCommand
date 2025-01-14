#ifndef LOGIC_H
#define LOGIC_H
#include <stdbool.h>
#include "tree.h"

int compare_entries(const struct dirent **a, const struct dirent **b);
void increment_counters(bool is_dir);
void process_directory(const char *path, int level, TreeNode *parent);
void *worker(void *arg);
const char *parse_options(int argc, char *argv[]);



#endif

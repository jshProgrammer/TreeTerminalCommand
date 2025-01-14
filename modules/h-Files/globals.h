#ifndef GLOBALS_H
#define GLOBALS_H
#include <sys/_pthread/_pthread_mutex_t.h>
#include "tree.h"

extern const int max_path;
#define MAX_INDENT 128
#define MAX_THREADS 16
#define MAX_PRUNED_DIRS 16

extern int option_show_full_path;
extern int option_max_depth;
extern int option_follow_symlinks;
extern int option_show_hidden;
extern int option_show_file_sizes;
extern int option_show_summary;
extern int option_dirs_first;
extern int option_reverse_sort;
extern int option_dirs_only;
extern int option_ignore_case;
extern int option_file_limit;
extern int option_sort_time;
extern int option_output_json;
extern int option_output_csv;
extern int option_show_user;
extern int option_show_group;
extern char *output_file;

extern int  total_files, total_dirs;
extern char *pruned_directories[MAX_PRUNED_DIRS];
extern int pruned_dir_count;

extern TreeNode *global_root;

extern pthread_mutex_t counter_mutex;
extern pthread_mutex_t tree_mutex;

#endif
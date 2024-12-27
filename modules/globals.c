#include "h-Files/globals.h"

#include <pthread.h>
#include <stddef.h>
#include <sys/_pthread/_pthread_mutex_t.h>

#include "h-Files/tree.h"


int option_show_full_path = 0;
int option_max_depth = -1;
int option_follow_symlinks = 0;
int option_show_hidden = 0;
int option_show_file_sizes = 0;
int option_show_summary = 1;
int option_dirs_first = 0;
int option_reverse_sort = 0;
int option_dirs_only = 0;
int option_ignore_case = 0;
int option_file_limit = -1;
int option_sort_time = 0;
int option_output_json = 0;
int option_output_csv = 0;
int option_show_user = 0;
int option_show_group = 0;
char *output_file = NULL;

int total_files = 0, total_dirs = 1;
char *pruned_directories[MAX_PRUNED_DIRS]; // Array to store pruned directories
int pruned_dir_count = 0;

const int max_path = 1024;
TreeNode *global_root = NULL;

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tree_mutex = PTHREAD_MUTEX_INITIALIZER;
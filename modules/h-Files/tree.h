#ifndef TREE_H
#define TREE_H
#define MAX_PATH
#include <pthread.h>

typedef struct TreeNode {
    char *name;
    long size;
    int level;
    int is_dir;
    struct TreeNode **children;
    int child_count;
} TreeNode;

TreeNode *create_node(const char *name, long size, int level, int is_dir);
void add_child(TreeNode *parent, TreeNode *child);
void free_tree(TreeNode *node);


#endif
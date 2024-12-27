#include <stdlib.h>
#include <string.h>
#include "h-Files/tree.h"
#include "h-Files/globals.h"
#include <stdio.h>

TreeNode *create_node(const char *name, long size, int level, int is_dir) {
    TreeNode *node = malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->size = size;
    node->level = level;
    node->is_dir = is_dir;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

void add_child(TreeNode *parent, TreeNode *child) {
    pthread_mutex_lock(&tree_mutex);

    parent->children = realloc(parent->children, (parent->child_count + 1) * sizeof(TreeNode *));
    parent->children[parent->child_count++] = child;

    pthread_mutex_unlock(&tree_mutex);
}

void free_tree(TreeNode *node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; ++i) {
        free_tree(node->children[i]);
    }
    free(node->children);
    free(node->name);
    free(node);
}


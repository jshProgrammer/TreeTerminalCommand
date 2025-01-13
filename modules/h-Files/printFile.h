#ifndef PRINTFILE_H
#define PRINTFILE_H
#include "tree.h"
#include "logic.h"
#include <stdio.h>


void print_indents_in_file(FILE *file, int indent);
void generate_json_output(FILE *file, TreeNode *node, int indent);
void generate_csv_output(FILE *file, TreeNode *node);
void output_to_txt_file(const char *path);

#endif

#include "global_variables.h"

#ifndef _AUX_FUNCTIONS_H
#define _AUX_FUNCTIONS_H

void Interface(node *node);
char *DeleteMethod(char *string);
char *DeleteMethod_space(char *string);
char *find_next_protocol(char *buffer);
int max(int x, int y, int z, int j);
void search_index(node *node, char node_id[], int index);
void check_for_new_node(node *node, char path[]);
void initialize_tables(node *node);
void update_neighbour(node *node, char old_neighbour[], char new_neighbour[]);
void update_tables(node *node);
void clean_node(node *node);

#endif
#ifndef _SELECT_H
#define _SELECT_H

#include "global_variables.h"

void impl_select(node *node);
void choose_terminal_cmd(node *node);
void choose_received_protocol(node *node, char buffer[], int fd);

#endif
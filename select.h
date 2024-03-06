#include "global_variables.h"

#ifndef _SELECT_H
#define _SELECT_H

void impl_select(char IP[], char TCP[], char regIP[], char regUDP[], node *node);
void choose_terminal_cmd(node *node);
void choose_received_protocol(node *node, char buffer[]);

#endif
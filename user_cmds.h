#include "udp_functions.h"
#include "tcp_functions.h"
#include "protocol_msgs.h"
#include "aux_functions.h"
#include "global_variables.h"

#ifndef _USER_COMMANDS_H
#define _USER_COMMANDS_H

void join(char ring[], node *n);
void direct_join(char ring[], node *n, char buffer[]);
void show_topology(node *node, int chord_bool);
void leave(node* n);
void show_routing(node *node, char dest[]);
void show_path(node *node, char dest[]);
void show_forwarding(node *node);
void chord(node *n, char buffer[]);

#endif
#include "global_variables.h"

#ifndef _PROTOCOL_MESSAGES_H
#define _PROTOCOL_MESSAGES_H

void NODES(char ring[], char answer[]);
void REG(char ring[], char answer[], node *node);
void UNREG(char ring[], char answer[], node *node);
int ENTRY(char connectIP[], char connectTCP[], node *node);
void ENTRY_not_me(char connectIP[], char connectTCP[], node *node, char aux_id[], char aux_IP[], char aux_TCP[]);
int PRED(char connectIP[], char connectTCP[], node *node, int sock_created);
void SUCC(char connectIP[], char connectTCP[], node *node, int sock_created);
void CHORD(char connectIP[], char connectTCP[], node *node);
void ROUTE(node *node, char target_node[]);
void receive_ENTRY();
void receive_ROUTE(node *node);
void initialize_tables(node *node);

#endif
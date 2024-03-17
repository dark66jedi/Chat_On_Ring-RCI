#include "global_variables.h"

#ifndef _PROTOCOL_MESSAGES_H
#define _PROTOCOL_MESSAGES_H

void NODES(char ring[], char answer[]);
void REG(char ring[], char answer[], node *node);
void UNREG(char ring[], char answer[], node *node);
int ENTRY(char connectIP[], char connectTCP[], node *node);
void ENTRY_not_me(char connectIP[], char connectTCP[], node *node, char aux_id[], char aux_IP[], char aux_TCP[]);
int PRED(char connectIP[], char connectTCP[], node *node, int sock_created);
int SUCC(char connectIP[], char connectTCP[], node *node, int sock_created);
int CHORD(char connectIP[], char connectTCP[], node *node);
void CHAT(node *node, char i[], char n[], char message[]);
void ROUTE(node *node, char target_node[], int fd);
void receive_ENTRY();
void ROUTE_Received(node *node, char origin_id[], char dest_id[], char path[]);
void initialize_tables(node *node);

#endif
#include "global_variables.h"

#ifndef _PROTOCOL_MSGS_H
#define _PROTOCOL_MSGS_H

void NODES(char ring[], char answer[]);
void REG(char ring[], char answer[], node *node);
void UNREG(char ring[], char answer[], node *node);
int ENTRY(char connectIP[], char connectTCP[], node *node);
void ENTRY_not_me(char connectIP[], char connectTCP[], node *node, char aux_id[], char aux_IP[], char aux_TCP[]);
void ENTRY_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd);
int PRED(char connectIP[], char connectTCP[], node *node, int sock_created);
void PRED_Received(char aux_id[], node *node, int fd);
int SUCC(char connectIP[], char connectTCP[], node *node, int sock_created);
void SUCC_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd);
int CHORD(char connectIP[], char connectTCP[], node *node);
void CHORD_Received(char aux_id[], node *node, int fd);
void ROUTE(node *node, char target_node[], int fd);
void ROUTE_Received(node *node, char origin_id[], char dest_id[], char path[]);
void CHAT(node *node, char origin[], char dest[], char message[]);
void CHAT_Received(node *node, char i[], char n[], char message[]);

#endif
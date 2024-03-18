#ifndef _SELECT_H
#define _SELECT_H

#include "global_variables.h"

void impl_select(node *node);
void choose_terminal_cmd(node *node, char readInter[]);
void choose_received_protocol(node *node, char buffer[], int fd);
void SUCC_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd);
void ENTRY_Received(char aux_id[], char aux_IP[], char aux_TCP[], node *node, int fd);
void PRED_Received(char aux_id[], node *node, int fd);
void CHORD_Received(char aux_id[], node *node, int fd);
void CHAT_Received(node *node, char i[], char n[], char message[]);

#endif
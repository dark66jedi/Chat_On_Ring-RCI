#include "global_variables.h"

#ifndef _PROTOCOL_MESSAGES_H
#define _PROTOCOL_MESSAGES_H

void NODES(char ring[], char answer[]);
void REG(char ring[], char answer[], node *node);
void UNREG(char ring[], char answer[], node *node);
void ENTRY(char connectIP[], char connectTCP[], node *node);
void PRED(char connectIP[], char connectTCP[], node *node);
void CHORD(char connectIP[], char connectTCP[], node *node);
void receive_ENTRY();

#endif
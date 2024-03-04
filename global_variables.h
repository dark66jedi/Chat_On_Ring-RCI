#ifndef GLOBAL_VARIABLES_H_
#define GLOBAL_VARIABLES_H_

#define EXTERN extern

#ifdef DEFINE_GLOBAL_VARIABLES
    #define EXTERN     /*nothing*/
#else
    #define EXTERN     extern
#endif

EXTERN int x;
EXTERN char regIP[20];
EXTERN char regUDP[5];

#endif
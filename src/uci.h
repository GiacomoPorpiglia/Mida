#ifndef UCI_H
#define UCI_H


#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include "make_move.h"
#include "perft.h"
#include "hashing.h"
#include "search.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#endif
/*==================================\
|                                   |
|       Time controls variables     |
|              for UCI              |
\==================================*/

// exit from engine flag
extern int quit;

// UCI "movestogo" command moves counter
extern int movestogo;

// UCI "movetime" command time counter
extern int movetime;

// UCI "time" command holder (ms)
extern int my_time;

// UCI "inc" command's time increment holder
extern int inc;

// UCI "starttime" command time holder
extern int starttime;

// UCI "stoptime" command time holder
extern int stoptime;

// variable to flag time control availability
extern int timeset;

// variable to flag when the time is up
extern int stopped;

//static inline int gettimeofday(struct timeval *tp, struct timezone *tzp);

// get time in milliseconds
int get_time_ms();

int input_waiting();

void read_input();
// bridge function to interact between search and GUI input
void communicate();

// parse user/GUI move string input (e.g. "e7e8q")
static inline MOVE parse_move(char const *move_string);

void parse_position(char const *command);
void reset_time_control();

// parse UCI "go" command
static inline void parse_go(char *command);
void uci_loop();

#endif
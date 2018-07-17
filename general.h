// Shared includes and functions
// FROM solution 2, by Brian
#ifndef _GENERAL_H_
#define _GENERAL_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#define SLEEP_TIME_US 100

// Trigger an orderly shutdown
void General_shutdown(void);
_Bool General_stoppingProgram(void);

void die_on_failed(char *msg);
void sleep_usec(long usec);
void sleep_msec(long msec);


#endif

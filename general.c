// FROM solution 2, by Brian
#include "general.h"
#include <time.h>

static _Bool stoppingProgram = false;

void General_shutdown(void) {
	stoppingProgram = true;
}

_Bool General_stoppingProgram(void) {
	return stoppingProgram;
}

void sleep_usec(long usec) {
	struct timespec sleep_time;
	sleep_time.tv_sec = (usec / 1000000);
	sleep_time.tv_nsec = (usec % 1000000) * 1000;
	nanosleep(&sleep_time, NULL);
}

void sleep_msec(long msec) {
	sleep_usec(msec * 1000);
}

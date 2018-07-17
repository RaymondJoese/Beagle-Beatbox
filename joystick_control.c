// module to handle zen-cape joystick inputs

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "joystick_control.h"
#include "drum_modes.h"
#include "audioMixer.h"

#define EXPORT_GPIO "/sys/class/gpio/export"
#define JOY_IN_GPIO_27 27
#define JOY_UP_GPIO_26 26
#define JOY_RIGHT_GPIO_47 47
#define JOY_DOWN_GPIO_46 46
#define JOY_LEFT_GPIO_65 65

#define Joystick_IN_Value "/sys/class/gpio/gpio27/value"
#define Joystick_UP_Value "/sys/class/gpio/gpio26/value"
#define Joystick_RIGHT_Value "/sys/class/gpio/gpio47/value"
#define Joystick_DOWN_Value "/sys/class/gpio/gpio46/value"
#define Joystick_LEFT_Value "/sys/class/gpio/gpio65/value"

#define MAX_LENGTH 1024
#define ASLII_FOR_0 48
#define FIRST_BUFF_INDEX 0

#define DIRECTION_SELECTED 0
#define NANOSECOND_FOR_10MS 10000000
#define DELAY_BETWEEN_VOL_BPM_CHANGES_IN_NANOSEC 300000000

void* joystickControlThread(void* arg);
static _Bool joystickControlStopping = false;
static pthread_t joystickControlThreadId;

void JoystickControl_init(void) {
    pthread_create(&joystickControlThreadId, NULL, &joystickControlThread, NULL);
}

void JoystickControl_cleanup(void) {
    printf("Stopping joystick control thread...\n");

    joystickControlStopping = true;

    pthread_join(joystickControlThreadId, NULL);
    printf("Done stopping joystick control...\n");
}

void exportGpio(int gpio) {
    // Use fopen() to open the file for write access.
    FILE *pfile = fopen(EXPORT_GPIO, "w");
    if (pfile == NULL) {
          printf("ERROR: Unable to open export file.\n");
          exit(1);
    }
    // Write to data to the file using fprintf():
    int exported = fprintf(pfile, "%d", gpio);
    if (exported <= 0) {
        printf("ERROR WRITING DATA.\n");
        exit(-1);
    }
    // Close the file using fclose():
    fclose(pfile);
}

void loadJoystickGpios(void) {
    exportGpio(JOY_IN_GPIO_27);
    exportGpio(JOY_UP_GPIO_26);
    exportGpio(JOY_RIGHT_GPIO_47);
    exportGpio(JOY_DOWN_GPIO_46);
    exportGpio(JOY_LEFT_GPIO_65);
}

int readFromFile(char *fileName) {
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		printf("ERROR: Unable to open file (%s) for read\n", fileName);
		exit(-1);
	}
	// Read string (line)
	const int max_length = MAX_LENGTH;
	char buff[max_length];
	fgets(buff, max_length, file);

	// Close
	fclose(file);
	int result = buff[FIRST_BUFF_INDEX] - ASLII_FOR_0;
	return result;
}

enum direction checkJoystickDirection(void) {
	enum direction dir;
	dir = NO_DIRECTION;

	if (readFromFile(Joystick_UP_Value) == DIRECTION_SELECTED) {
		dir = UP;
	} else if (readFromFile(Joystick_RIGHT_Value) == DIRECTION_SELECTED){
		dir = RIGHT;
	} else if (readFromFile(Joystick_DOWN_Value) == DIRECTION_SELECTED) {
		dir = DOWN;
	} else if (readFromFile(Joystick_LEFT_Value) == DIRECTION_SELECTED) {
		dir = LEFT;
	} else if (readFromFile(Joystick_IN_Value) == DIRECTION_SELECTED) {
        dir = PRESS_IN;
    }
	return dir;
}

void* joystickControlThread(void* arg) {
    loadJoystickGpios();
    while (!joystickControlStopping) {
        enum direction dir = checkJoystickDirection();
        if (dir == PRESS_IN) {
            enum mode current_mode = DrumMode_getCurMode();
            if (current_mode == OFF_MODE ) {
                DrumMode_changCurMode(ROCK_DRUM_MODE);
            } else if (current_mode == ROCK_DRUM_MODE) {
                DrumMode_changCurMode(OTHER_MODE);
            } else if (current_mode == OTHER_MODE){
                DrumMode_changCurMode(OFF_MODE);
            }
            while (checkJoystickDirection() != NO_DIRECTION) {
                // printf("bad loop\n");
            } // wait till user release joystick
        } else if (dir == UP || dir == DOWN) {
            if (dir == UP) {
                AudioMixer_volumeUp();
            } else {
                AudioMixer_volumeDown();
            }
            sleepForNNanoSec(DELAY_BETWEEN_VOL_BPM_CHANGES_IN_NANOSEC);
        } else if (dir == LEFT || dir == RIGHT) {
            if (dir == LEFT) {
                DrumMode_bpmDown();
            } else {
                DrumMode_bpmUp();
            }
            sleepForNNanoSec(DELAY_BETWEEN_VOL_BPM_CHANGES_IN_NANOSEC);
        }
        sleepForNNanoSec(NANOSECOND_FOR_10MS);
    }
    return NULL;
}

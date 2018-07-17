#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "accel_control.h"
#include "drum_modes.h"
#include "general.h"
#include "audioMixer.h"

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

#define I2C_DEVICE_ADDRESS 0x1C

#define CTRL_REG1 0x2A
#define STATUS_REG 0x00
#define REG_XMSB 0x01
#define REG_XLSB 0x02
#define REG_YMSB 0x03
#define REG_YLSB 0x04
#define REG_ZMSB 0x05
#define REG_ZLSB 0x06

#define STANDBY 0x00
#define ACTIVE 0x01

#define NUM_BYTES_TO_READ 7
#define BYTE 8
#define TWO_BYTES 16

#define XY_THRESHOLD 1000
#define Z_THRESHOLD 1400

#define TWO_G 2047
#define BIAS 4096

#define PLAY_TIMES 1
#define SLEEP_TIME 100000


static _Bool accelControlStopping = false;
static pthread_t accelControlThreadId;


// phototype
void* accelControlThread(void* arg);
static int initI2cBus(char* bus, int address);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);

void AccelControl_init(void) {
    pthread_create(&accelControlThreadId, NULL, &accelControlThread, NULL);
}

void AccelControl_cleanup(void) {
    printf("Stopping accel_control thread...\n");

    accelControlStopping = true;

    pthread_join(accelControlThreadId, NULL);
    printf("Done stopping accel_control thread...\n");
}

// From I2C guide, initialize I2C bus and function that write I2C register
static int initI2cBus(char* bus, int address) {
    int i2cFileDesc = open(bus, O_RDWR);
    if (i2cFileDesc < 0) {
        printf("I2C: Unable to open bus for read/write (%s)\n", bus);
        perror("Error is:");
        exit(1);
    }
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    return i2cFileDesc;
}
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value) {
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2) {
        perror("I2C: Unable to write i2c register.");
        exit(1);
    }
}

void* accelControlThread(void* arg){
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    // Active device
    writeI2cReg(i2cFileDesc, CTRL_REG1, STANDBY);
    writeI2cReg(i2cFileDesc, CTRL_REG1, ACTIVE);
    // load sounds
	wavedata_t bassdrumFile, hiHatFile, snareFile;
	AudioMixer_readWaveFileIntoMemory(BASSDRUM, &bassdrumFile);
	AudioMixer_readWaveFileIntoMemory(HI_HAT, &hiHatFile);
	AudioMixer_readWaveFileIntoMemory(SNARE_SOFT, &snareFile);

    while (!accelControlStopping) {
        // Read 7 bytes of data from 0x00 to 0x06
        char buff[NUM_BYTES_TO_READ];
        if (read(i2cFileDesc, buff, NUM_BYTES_TO_READ) != NUM_BYTES_TO_READ) {
            printf("Error : Input/Output error \n");
        } else {
            // Convertion from https://github.com/ControlEverythingCommunity/MMA8452Q/blob/master/C/MMA8452Q.c
            // Convert the data to 12-bits
            int x = ((buff[REG_XMSB] << BYTE) | buff[REG_XLSB]) / TWO_BYTES;
            if(x > TWO_G) {
                x -= BIAS;
            }

            int y = ((buff[REG_YMSB] << BYTE) | buff[REG_YLSB]) / TWO_BYTES;
            if(y > TWO_G) {
                y -= BIAS;
            }

            int z = ((buff[REG_ZMSB] << BYTE) | buff[REG_ZLSB]) / TWO_BYTES;
            if(z > TWO_G) {
                z -= BIAS;
            }

            if (x >= XY_THRESHOLD || x <= -XY_THRESHOLD) {
                AudioMixer_queueSound(&snareFile);
            }
            if (y >= XY_THRESHOLD || y <= -XY_THRESHOLD) {
                AudioMixer_queueSound(&hiHatFile);
            }
            if (z >= Z_THRESHOLD || z <= -Z_THRESHOLD) {
                AudioMixer_queueSound(&bassdrumFile);
            }
    		// printf("X-Axis : %d \n", x);
    		// printf("Y-Axis : %d \n", y);
    		// printf("Z-Axis : %d \n\n", z);
        }
        // sleep(1);
        sleep_usec(SLEEP_TIME);

    }

    // free sounds:
	AudioMixer_freeWaveFileData(&bassdrumFile);
	AudioMixer_freeWaveFileData(&hiHatFile);
	AudioMixer_freeWaveFileData(&snareFile);
    return NULL;
}

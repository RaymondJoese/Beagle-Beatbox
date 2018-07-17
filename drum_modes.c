// module to switch different playing modes. available modes include OFF_MODE, ROCK_DRUM_MODE, OTHER_MODE

#include <alsa/asoundlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include "audioMixer.h"
#include "drum_modes.h"

// File used for play-back:
// If cross-compiling, must have this file available, via this relative path,
// on the target when the application is run. This example's Makefile copies the wave-files/
// folder along with the executable to ensure both are present.

#define BASSDRUM "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"
#define HI_HAT "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define SNARE_SOFT "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
// #define SNARE_HARD "beatbox-wav-files/100058__menegass__gui-drum-snare-hard.wav"
// #define DRUMTOMMID_HARD "beatbox-wav-files/100066__menegass__gui-drum-tom-mid-hard.wav"
#define DRUMTOMMID_SOFT "beatbox-wav-files/100067__menegass__gui-drum-tom-mid-soft.wav"
// #define DRUMCYC_SOFT "beatbox-wav-files/100057__menegass__gui-drum-cyn-soft.wav"
// #define DURMTOMLOW_HARD "beatbox-wav-files/100064__menegass__gui-drum-tom-lo-hard.wav"
#define DURMTOMLOW_SOFT "beatbox-wav-files/100065__menegass__gui-drum-tom-lo-soft.wav"
// #define DURMTOMHI_HARD "beatbox-wav-files/100062__menegass__gui-drum-tom-hi-hard.wav"
#define DURMTOMHI_SOFT "beatbox-wav-files/100063__menegass__gui-drum-tom-hi-soft.wav"
#define SPLASH_SOFT "beatbox-wav-files/100061__menegass__gui-drum-splash-soft.wav"


#define SAMPLE_RATE   44100
#define NUM_CHANNELS  1
#define SAMPLE_SIZE   (sizeof(short)) 	// bytes per sample

#define DEFAULT_BPM 120

#define SECONDS_PER_MINUE 60
#define DIVIDER_HALF 2
#define SEC_TO_NANOSEC 1000000000
#define PLAY_SOUND_DELAY 900000000
#define OFF_MODE_THREAD_SLEEP_TIME 100000

#define START_HALF_BEAT 0
#define HALF_BEAT_1 1
#define HALF_BEAT_2 2
#define HALF_BEAT_3 3
#define HALF_BEAT_4 4
#define HALF_BEAT_5 5
#define HALF_BEAT_6 6
#define HALF_BEAT_7 7
#define MAX_HALF_BEAT 8
// special case
#define HALF_BEAT_8 8
#define HALF_BEAT_9 9
#define HALF_BEAT_10 10
#define HALF_BEAT_11 11
#define HALF_BEAT_12 12
#define HALF_BEAT_13 13
#define HALF_BEAT_14 14
#define HALF_BEAT_14 14
#define HALF_BEAT_15 15

#define DEFAULT_SEPARATER -1
#define SEPARATER_0 0
#define SEPARATER_1 1
#define SEPARATER_2 2
#define SEPARATER_3 3
#define SEPARATER_4 4

#define MULTIPLER_4_BEATS 2

static int BPM = 0;

static enum mode cur_mode = OFF_MODE;

void* drumModesThread(void* arg);
static _Bool drumModeStopping = false;
static pthread_t drumModesThreadId;
// static pthread_mutex_t modeMutex = PTHREAD_MUTEX_INITIALIZER;


void DrumMode_init(void) {
	DrumMode_setBPM(DEFAULT_BPM);

	pthread_create(&drumModesThreadId, NULL, &drumModesThread, NULL);
}

void DrumMode_Cleanup(void) {
	printf("Stopping drum mode thread...\n");

	drumModeStopping = true;
	pthread_join(drumModesThreadId, NULL);
	printf("Done stopping mode...\n");
}

void DrumMode_playSound(char* fileName, int times) {
	wavedata_t wavedata;
	AudioMixer_readWaveFileIntoMemory(fileName, &wavedata);
	for (int i = 0; i < times; i++) {
		AudioMixer_queueSound(&wavedata);
		sleepForNNanoSec(PLAY_SOUND_DELAY);
	}
	AudioMixer_freeWaveFileData(&wavedata);
}

long nanoSecForHalfBeat(int BPM) {
	// Time For Half Beat [sec] = 60 [sec/min] / BPM / 2 [half-beats per beat]
	return SECONDS_PER_MINUE / (double) BPM / DIVIDER_HALF * SEC_TO_NANOSEC;
}

void sleepForNNanoSec(long n) {
	long seconds = 0;
	long nanoseconds = n;
	struct timespec reqDelay = {seconds, nanoseconds};
	nanosleep(&reqDelay, (struct timespec *) NULL);
}

enum mode DrumMode_getCurMode(void) {
	return cur_mode;
}

const char* DrumMode_getCurModeForPrint(void) {
	switch (cur_mode) {
		case OFF_MODE: return "OFF";
		case ROCK_DRUM_MODE: return "Rock Drum";
		case OTHER_MODE: return "Other";
		default: return "Unknown Mode?";
	}
}

void DrumMode_changCurMode(enum mode newMode) {
	cur_mode = newMode;
	printf("cur_mode = %s\n", DrumMode_getCurModeForPrint());
}

int DrumMode_getBPM (void) {
	return BPM;
}

void DrumMode_setBPM(int newBPM) {
	if (newBPM > DRUMMODE_MAX_BPM) {
		printf("ERROR: max BPM is %d\n", DRUMMODE_MAX_BPM);
		BPM = DRUMMODE_MAX_BPM;
	} else if (newBPM < DRUMMODE_MIN_BPM) {
		printf("ERROR: min BPM is %d\n", DRUMMODE_MIN_BPM);
		BPM = DRUMMODE_MIN_BPM;
	} else {
		BPM = newBPM;
	}
}

void DrumMode_bpmUp(void) {
	int newBPM = BPM + DRUMMODE_BPM_PER_CHANGE;
	if (DRUMMODE_MIN_BPM <= newBPM && newBPM <= DRUMMODE_MAX_BPM) {
		DrumMode_setBPM(newBPM);
	}
}

void DrumMode_bpmDown(void) {
	int newBPM = BPM - DRUMMODE_BPM_PER_CHANGE;
	if (DRUMMODE_MIN_BPM <= newBPM && newBPM <= DRUMMODE_MAX_BPM) {
		DrumMode_setBPM(newBPM);
	}
}

void startFourBeats(wavedata_t* tomHiSoft, wavedata_t* snare, wavedata_t* tomMidSoft) {
	int half_beat = START_HALF_BEAT;
	while (!drumModeStopping && half_beat < MAX_HALF_BEAT && cur_mode == OTHER_MODE ) {
		if (half_beat == START_HALF_BEAT) {
			AudioMixer_queueSound(tomHiSoft);
		} else if (half_beat == HALF_BEAT_7) {
			AudioMixer_queueSound(tomMidSoft);
		} else {
			AudioMixer_queueSound(snare);
		}
		half_beat++;
		sleepForNNanoSec(nanoSecForHalfBeat(BPM));
	}
}

void mainEightBeats(wavedata_t* splash, wavedata_t* bass, wavedata_t* hihat, wavedata_t* snare) {
	int half_beat = START_HALF_BEAT;
	while (!drumModeStopping && half_beat < MULTIPLER_4_BEATS * MAX_HALF_BEAT && cur_mode == OTHER_MODE) {
		if (half_beat == START_HALF_BEAT) {
			AudioMixer_queueSound(splash);
			AudioMixer_queueSound(bass);
		} else {
			AudioMixer_queueSound(hihat);
			if (half_beat == HALF_BEAT_2 || half_beat == HALF_BEAT_6 || half_beat == HALF_BEAT_10 || half_beat == HALF_BEAT_14) {
				AudioMixer_queueSound(snare);
			} else if (half_beat == HALF_BEAT_4 || half_beat ==  HALF_BEAT_5 || half_beat == HALF_BEAT_8 || half_beat == HALF_BEAT_12 || half_beat == HALF_BEAT_13) {
				AudioMixer_queueSound(bass);
			}
		}
		half_beat++;
		sleepForNNanoSec(nanoSecForHalfBeat(BPM));
	}
}

void firstSeparater (wavedata_t* snare, wavedata_t* hihat) {
	int half_beat = START_HALF_BEAT;
	while (!drumModeStopping && half_beat < HALF_BEAT_3 && cur_mode == OTHER_MODE) {
		AudioMixer_queueSound(snare);
		if (half_beat == START_HALF_BEAT) {
			AudioMixer_queueSound(hihat);
		}
		half_beat++;
		sleepForNNanoSec(nanoSecForHalfBeat(BPM));
	}
}

void secondSeparater(wavedata_t* snare, wavedata_t* bass, wavedata_t* tomLowHard) {
	int half_beat = START_HALF_BEAT;
	while (!drumModeStopping && half_beat < MAX_HALF_BEAT && cur_mode == OTHER_MODE) {
		AudioMixer_queueSound(snare);
		AudioMixer_queueSound(bass);
		AudioMixer_queueSound(tomLowHard);
		half_beat++;
		sleepForNNanoSec(nanoSecForHalfBeat(BPM));
	}
}

void thirdSeparater(wavedata_t* snare, wavedata_t* tomHiSoft, wavedata_t* tomMidSoft) {
	int half_beat = START_HALF_BEAT;
	while (!drumModeStopping && half_beat < HALF_BEAT_4 && cur_mode == OTHER_MODE) {
		if (half_beat == START_HALF_BEAT) {
			AudioMixer_queueSound(snare);
		} else if (half_beat == HALF_BEAT_1 || half_beat == HALF_BEAT_2) {
			AudioMixer_queueSound(tomHiSoft);
		} else {
			AudioMixer_queueSound(tomMidSoft);
		}
		half_beat++;
		sleepForNNanoSec(nanoSecForHalfBeat(BPM));
	}
}

void fourthSeparater(wavedata_t* snare, wavedata_t* bass) {
	int half_beat = START_HALF_BEAT;
	while (!drumModeStopping && half_beat < HALF_BEAT_6 && cur_mode == OTHER_MODE) {
		if (half_beat == START_HALF_BEAT || half_beat == HALF_BEAT_2 || half_beat == HALF_BEAT_4 || half_beat == HALF_BEAT_6) {
			AudioMixer_queueSound(snare);
		} else {
			AudioMixer_queueSound(bass);
		}
		half_beat++;
		sleepForNNanoSec(nanoSecForHalfBeat(BPM));
	}
}


void* drumModesThread(void* arg) {
	AudioMixer_init();

	wavedata_t tomMidSoftFile, splashSoftFile, tomLowSoftFile, tomHiSoftFile, bassdrumFile, snareSoftFile, hiHatFile;
	AudioMixer_readWaveFileIntoMemory(DRUMTOMMID_SOFT, &tomMidSoftFile);
	AudioMixer_readWaveFileIntoMemory(DURMTOMLOW_SOFT, &tomLowSoftFile);
	AudioMixer_readWaveFileIntoMemory(DURMTOMHI_SOFT, &tomHiSoftFile);
	AudioMixer_readWaveFileIntoMemory(SPLASH_SOFT, &splashSoftFile);
	AudioMixer_readWaveFileIntoMemory(BASSDRUM, &bassdrumFile);
	AudioMixer_readWaveFileIntoMemory(HI_HAT, &hiHatFile);
	AudioMixer_readWaveFileIntoMemory(SNARE_SOFT, &snareSoftFile);

	while (!drumModeStopping) {
		int half_beat = HALF_BEAT_1;
		int separater = SEPARATER_0;
		switch (cur_mode) {
			case OFF_MODE:
				sleepForNNanoSec(OFF_MODE_THREAD_SLEEP_TIME);
				break;
			case ROCK_DRUM_MODE:
				while (!drumModeStopping && cur_mode == ROCK_DRUM_MODE) {
					long n = nanoSecForHalfBeat(BPM);
					AudioMixer_queueSound(&hiHatFile);

					if (half_beat == HALF_BEAT_1 || half_beat == HALF_BEAT_5) {
						AudioMixer_queueSound(&bassdrumFile);
					} else if (half_beat == HALF_BEAT_3 || half_beat == HALF_BEAT_7) {
						AudioMixer_queueSound(&snareSoftFile);
					} else if (half_beat == MAX_HALF_BEAT) {
						half_beat = START_HALF_BEAT;
					}
					half_beat++;
					sleepForNNanoSec(n);
				}
				break;
			case OTHER_MODE:
				while (!drumModeStopping && cur_mode == OTHER_MODE) {
					if (separater == SEPARATER_0) {
						startFourBeats(&tomHiSoftFile, &snareSoftFile, &tomMidSoftFile);
					} else if (separater == SEPARATER_1) {
						firstSeparater(&snareSoftFile, &hiHatFile);
					} else if (separater == SEPARATER_2) {
						secondSeparater(&snareSoftFile, &bassdrumFile, &tomLowSoftFile);
					} else if (separater == SEPARATER_3) {
						thirdSeparater(&snareSoftFile, &tomHiSoftFile, &tomMidSoftFile);
					} else if (separater == SEPARATER_4) {
						fourthSeparater(&snareSoftFile, &bassdrumFile);
						separater = DEFAULT_SEPARATER;
					}
					mainEightBeats(&splashSoftFile, &bassdrumFile, &hiHatFile, &snareSoftFile);
					separater++;
				}
				break;
			default:
				printf("ERROR: Unknown mode\n");
		}
	}

	AudioMixer_freeWaveFileData(&bassdrumFile);
	AudioMixer_freeWaveFileData(&hiHatFile);
	AudioMixer_freeWaveFileData(&snareSoftFile);
	AudioMixer_freeWaveFileData(&tomMidSoftFile);
	AudioMixer_freeWaveFileData(&tomLowSoftFile);
	AudioMixer_freeWaveFileData(&tomHiSoftFile);
	AudioMixer_freeWaveFileData(&splashSoftFile);

	AudioMixer_cleanup();
	return NULL;
}

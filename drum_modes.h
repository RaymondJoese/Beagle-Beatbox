// module to switch different playing modes. available modes include OFF, ROCK_DRUM_MODE, OTHER_MODE
#ifndef DRUM_MODES_H
#define DRUM_MODES_H

#define BASSDRUM "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"
#define HI_HAT "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define SNARE_SOFT "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
#define DURMTOMHI_SOFT "beatbox-wav-files/100063__menegass__gui-drum-tom-hi-soft.wav"
#define DRUMTOMMID_SOFT "beatbox-wav-files/100067__menegass__gui-drum-tom-mid-soft.wav"
#define DURMTOMLOW_SOFT "beatbox-wav-files/100065__menegass__gui-drum-tom-lo-soft.wav"
#define SPLASH_SOFT "beatbox-wav-files/100061__menegass__gui-drum-splash-soft.wav"

#define DRUMMODE_MAX_BPM 300
#define DRUMMODE_MIN_BPM 40
#define DRUMMODE_BPM_PER_CHANGE 5

enum mode{OFF_MODE, ROCK_DRUM_MODE, OTHER_MODE};

// Run/Stop the thread
void DrumMode_init(void);
void DrumMode_Cleanup(void);

void DrumMode_playSound(char* fileName, int times);

void sleepForNNanoSec(long);

// getter and setter
const char* DrumMode_getCurModeForPrint(void);
enum mode DrumMode_getCurMode(void);
void DrumMode_changCurMode(enum mode newMode);

int DrumMode_getBPM (void);
void DrumMode_setBPM(int newBPM);
// bpm up/down for 5 units
void DrumMode_bpmUp(void);
void DrumMode_bpmDown(void);

#endif

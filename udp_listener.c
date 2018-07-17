// UDP Listening program on port 12345

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdbool.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include "udp_listener.h"
#include "joystick_control.h"
#include "drum_modes.h"
#include "audioMixer.h"
#include "general.h"

#define COMMAND_HELP        "help"
#define COMMAND_GET			"get"
#define COMMAND_INFO	  	"info"
#define COMMAND_MODE      	"mode"
#define COMMAND_ROCK		"rock"
#define COMMAND_OFF			"off"
#define COMMAND_OTHER		"other"
#define COMMAND_VOLUME		"vol"
#define COMMAND_BPM_C		"BPM"
#define COMMAND_bpm_L		"bpm"
#define COMMAND_ALL			"all" // for web socket
#define COMMAND_UP			"up"
#define COMMAND_DOWN		"down"
#define COMMAND_PLAY		"play"
#define COMMAND_STOP		"stop"
#define UNKNOWN_COMMAND 	"Unknown Command. Type 'help' for command list.\n\n"

#define MIN_SOUND_PLAY_TIMES 1
// #define MAX_SOUND_PLAY_TIMES 20
#define SOUND_1		1
#define SOUND_2		2
#define SOUND_3		3
#define SOUND_4		4
#define SOUND_5		5
#define SOUND_6		6
#define SOUND_7		7

int tokenizeCommand(char *buff, char *tokens[]);
int readCommand(char *buff, char *tokens[]);
void* UDP_listenerThread(void* arg);
static pthread_t udpListenerThreadId;


void UdpListener_init(void) {
	pthread_create(&udpListenerThreadId, NULL, &UDP_listenerThread, NULL);
}

void UdpListener_cheanup(void) {
	pthread_join(udpListenerThreadId, NULL);
}

int tokenizeCommand(char *buff, char *tokens[]) {
	int token_count = 0;

	// flag if start fetching token from buff
	_Bool flag_token_started = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH); // avoid overflows
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
			// find split point (space, tab, newline...)
			case ' ':
			case '\t':
			case '\n':
				// split buff by changing space/tab/newline to \0
				buff[i] = '\0';
				// indicate one token is splited from buff
				flag_token_started = false;
				break;
			default:
				// if not fetch; then fetch by let pointer of the
				//nth token points to the first letter (after space/tab/newline)
				if (!flag_token_started) {
					tokens[token_count] = &buff[i];
					token_count++;
					flag_token_started = true;
				}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

int readCommand(char *buff, char *tokens[]) {
	int length = strnlen(buff, COMMAND_LENGTH);
	if (length <= 0) {
		perror("ERROR: empty buffer\n");
		exit(-1);
	}

	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenizeCommand(buff, tokens);
	return token_count;
}

void* UDP_listenerThread(void * arg) {
	char *tokens[NUM_TOKENS];

	printf("Connect using: \n");
	printf("    netcat -u 192.168.7.2 %d\n", PORT);

	// Buffer to hold packet data:
	char message[COMMAND_LENGTH];

	// Address
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;                   // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network long
	sin.sin_port = htons(PORT);                 // Host to Network short

	// Create the socket for UDP
	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

	// Bind the socket to the port (PORT) that we specify
	bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

	char replyBuffer[REPLY_BUFFER_SIZE];

	while (!General_stoppingProgram()) {
		// Get the data (blocking)
		// Will change sin (the addtokenss) to be the addtokenss of the client.
		// Note: sin passes information in and out of call!
		unsigned int sin_len = sizeof(sin);
		int bytesRx = recvfrom(socketDescriptor, message, COMMAND_LENGTH, 0,
				(struct sockaddr *) &sin, &sin_len);

		// Make it null terminated (so string functions work):
		// NOTE: Unsafe in some cases; why?
		message[bytesRx] = 0;
		printf("Message received (%d bytes): \n\n'%s'\n", bytesRx, message);

		int token_count = readCommand(message, tokens);

		for (int i = 0; i < token_count; i++) {
			printf("token[%d] = %s\n", i, tokens[i]);
		}

		sin_len = sizeof(sin);

		if (tokens[0]) {
			if (strcmp(tokens[0], COMMAND_HELP) == 0) {
				sprintf(message, "--- Accepted command examples: -------------------------------\n");
				strcat(message, "get info	-- display all information.\n");
				strcat(message, "get mode	-- display the current playing mode.\n");
				strcat(message, "get vol		-- display current volume.\n");
				strcat(message, "get BPM		-- display current tempo in BPM.\n");

				strcat(message, "mode rock	-- change current mode to Standard Rock Drum.\n");
				strcat(message, "vol 80		-- change current volume to 80; range [0, 100].\n");
				strcat(message, "BPM 120		-- change current BPM to 120; range [40, 300].\n");
				strcat(message, "play 1		-- Play sound 1 once. 'get info' for details\n");
				strcat(message, "play 1	5	-- Play sound 1 five times.\n");
				strcat(message, "stop		-- cause the server program to end.\n");
				strcat(message, "---------------------------------------------------------------\n\n");
				sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
			} else if (strcmp(tokens[0], COMMAND_GET) == 0) {
				if (tokens[1]) {
					if (strcmp(tokens[1], COMMAND_INFO) == 0) {
						sprintf(message, "** Info *****************************************\n");
						strcat(message, "All available modes:\n");
						strcat(message, "OFF:       'off'\n");
						strcat(message, "Rock Drum: 'rock'\n");
						strcat(message, "Other:     'other'\n");
						strcat(message, "-------------------------------------------\n");
						strcat(message, "Available sounds: \n1. Bass Drum\n2. Hi-Hat\n3. Snare (soft)\n4. Tom Hi (soft)\n5. Tom Mid (soft)\n6. Tom Low (soft)\n7. Splash (Crash) (soft)\n");
						strcat(message, "-------------------------------------------\n");
						strcat(message, "Playing Mode: \t");
						strcat(message, DrumMode_getCurModeForPrint());
						strcat(message, "\nVolume: ");
						sprintf(replyBuffer, "\t%d\n", AudioMixer_getVolume());
						strcat(message, replyBuffer);
						strcat(message, "Tempo in BPM: ");
						sprintf(replyBuffer, "\t%d\n", DrumMode_getBPM());
						strcat(message, replyBuffer);
						strcat(message, "*************************************************\n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (strcmp(tokens[1], COMMAND_MODE) == 0) {
						sprintf(message, "Current Mode: ");
						strcat(message, DrumMode_getCurModeForPrint());
						strcat(message, " \n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (strcmp(tokens[1], COMMAND_VOLUME) == 0) {
						//sprintf(message, "Current Volume: ");
						sprintf(message, "Current Vol: %d \n\n", AudioMixer_getVolume());
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (strcmp(tokens[1], COMMAND_BPM_C) == 0 || strcmp(tokens[1], COMMAND_bpm_L) == 0)  {
						// sprintf(message, "Current BPM: ");
						sprintf(message, "Current BPM: %d \n\n", DrumMode_getBPM());
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (strcmp(tokens[1], COMMAND_ALL) == 0) {
						sprintf(message, "ALL: %d %d ", AudioMixer_getVolume(), DrumMode_getBPM());
						strcat(message, DrumMode_getCurModeForPrint());
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else {
						sprintf(message, UNKNOWN_COMMAND);
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					}
				} else {
					sprintf(message, UNKNOWN_COMMAND);
					sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
				}
			} else if (strcmp(tokens[0], COMMAND_MODE) == 0) { // change mode
				if (tokens[1]) {
					if (strcmp(tokens[1], COMMAND_OFF) == 0) {
						DrumMode_changCurMode(OFF_MODE);
					} else if (strcmp(tokens[1], COMMAND_ROCK) == 0) {
						DrumMode_changCurMode(ROCK_DRUM_MODE);
					} else if (strcmp(tokens[1], COMMAND_OTHER) == 0) {
						DrumMode_changCurMode(OTHER_MODE);
					} else {
						sprintf(message, "Unknown Mode. Please check all modes by 'get info'");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					}
					sprintf(message, "Mode is changed to: %s\n\n", DrumMode_getCurModeForPrint());
					sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
				} else { // tokens[1] == NULL
					sprintf(message, "Unknown Mode. Please check all modes by 'get info'\n\n");
					sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
				}
			} else if (strcmp(tokens[0], COMMAND_VOLUME) == 0) {
				if (tokens[1]) {
					if (strcmp(tokens[1], COMMAND_UP) == 0) {
						AudioMixer_volumeUp();
						sprintf(message, "Volume is changed to: %d\n\n", AudioMixer_getVolume());
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (strcmp(tokens[1], COMMAND_DOWN) == 0) {
						AudioMixer_volumeDown();
						sprintf(message, "Volume is changed to: %d\n\n", AudioMixer_getVolume());
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else {
						int newVolume = atoi(tokens[1]);
						if (AUDIOMIXER_MIN_VOLUME <= newVolume && newVolume <= AUDIOMIXER_MAX_VOLUME) {
							AudioMixer_setVolume(newVolume);
							sprintf(message, "Volume is changed to: %d\n\n", AudioMixer_getVolume());
							sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						} else {
							sprintf(message, "ERROR: Volume must be in range [0, 100].\n\n");
							sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						}
					}
				} else {
					sprintf(message, UNKNOWN_COMMAND);
					sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
				}
			} else if (strcmp(tokens[0], COMMAND_BPM_C) == 0 || strcmp(tokens[0], COMMAND_bpm_L) == 0) {
				if (tokens[1]) {
					if (strcmp(tokens[1], COMMAND_UP) == 0) {
						DrumMode_bpmUp();
						sprintf(message, "BPM is changed to: %d\n\n", DrumMode_getBPM());
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (strcmp(tokens[1], COMMAND_DOWN) == 0) {
						DrumMode_bpmDown();
						sprintf(message, "BPM is changed to: %d\n\n", DrumMode_getBPM());
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else {
					int newBPM = atoi(tokens[1]);
						if (DRUMMODE_MIN_BPM <= newBPM && newBPM <= DRUMMODE_MAX_BPM) {
							DrumMode_setBPM(newBPM);
							sprintf(message, "BPM is changed to: %d\n\n", DrumMode_getBPM());
							sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						} else {
							sprintf(message, "ERROR: BPM must be in range [40, 300].\n\n");
							sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						}
					}
				} else {
					sprintf(message, UNKNOWN_COMMAND);
					sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
				}
			} else if (strcmp(tokens[0], COMMAND_PLAY) == 0) {
				if (tokens[1]) {
					int soundNum = atoi(tokens[1]);
					// find # of times play
					int playTimes = MIN_SOUND_PLAY_TIMES;
					if (tokens[2] && SOUND_1 <= soundNum && soundNum <= SOUND_7) {
						int costomTimes = atoi(tokens[2]);
						if (MIN_SOUND_PLAY_TIMES <= costomTimes /* && costomTimes <= MAX_SOUND_PLAY_TIMES */) {
							playTimes = costomTimes;
						} else {
							sprintf(message, "Play times must be >= %d. Sound will be played once\n", MIN_SOUND_PLAY_TIMES);
							sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						}
					}
					//// if playing sound will change mode to OFF.
					// if (SOUND_1 <= soundNum && soundNum <= SOUND_7) {
					// 	// DrumMode_changCurMode(OFF_MODE);
					// }
					if (soundNum == SOUND_1) {
						sprintf(message, "Playing Bass Drum for %d times\n", playTimes);
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						DrumMode_playSound(BASSDRUM, playTimes);
						sprintf(message, "Done playing Bass Drum\n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (soundNum == SOUND_2) {
						sprintf(message, "Playing Hi-Hat for %d times\n", playTimes);
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						DrumMode_playSound(HI_HAT, playTimes);
						sprintf(message, "Done playing Hi-Hat\n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (soundNum == SOUND_3) {
						sprintf(message, "Playing Snare (soft) for %d times\n", playTimes);
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						DrumMode_playSound(SNARE_SOFT, playTimes);
						sprintf(message, "Done playing Snare\n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (soundNum == SOUND_4) {
						sprintf(message, "Playing Tom Hi (soft) for %d times\n", playTimes);
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						DrumMode_playSound(DURMTOMHI_SOFT, playTimes);
						sprintf(message, "Done playing Tom Hi\n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (soundNum == SOUND_5) {
						sprintf(message, "Playing Tom Mid (soft) for %d times\n", playTimes);
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						DrumMode_playSound(DRUMTOMMID_SOFT, playTimes);
						sprintf(message, "Done playing Tom Mid\n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (soundNum == SOUND_6) {
						sprintf(message, "Playing Tom Lo (soft) for %d times\n", playTimes);
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						DrumMode_playSound(DURMTOMLOW_SOFT, playTimes);
						sprintf(message, "Done playing Tom Lo\n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else if (soundNum == SOUND_7) {
						sprintf(message, "Playing Splash (soft) for %d times\n", playTimes);
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
						DrumMode_playSound(SPLASH_SOFT, playTimes);
						sprintf(message, "Done playing Splash\n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					} else {
						sprintf(message, "Unknown Sound. 'get info' for available sounds\n\n");
						sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					}
				} else {
					sprintf(message, "Unknown Sound. 'get info' for available sounds\n\n");
					sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
				}
			} else if (strcmp(tokens[0], COMMAND_STOP) == 0) {
				printf("Program terminating!\n");
				General_shutdown();
			} else {
				sprintf(message, UNKNOWN_COMMAND);
				sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
			}
		} else {
			sprintf(message, UNKNOWN_COMMAND);
			sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
		}
	}
	// Close
	close(socketDescriptor);
	pthread_exit(0);
}


//
// int main() {
// 	DrumMode_init();
// 	JoystickControl_init();
//
// 	pthread_t pid;
// 	pthread_create(&pid, NULL, UDP_listenerThread, NULL);
// 	pthread_join(pid, NULL);
//
// 	DrumMode_Cleanup();
// 	JoystickControl_cleanup();
//
// 	return 0;
// }

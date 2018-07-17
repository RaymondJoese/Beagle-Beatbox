#include <stdio.h>
#include <unistd.h>
// #include <netdb.h>
// #include <stdbool.h>
// #include <string.h>			// for strncmp()
// #include <unistd.h>			// for close()
// #include "audioMixer.h"
#include "udp_listener.h"
#include "joystick_control.h"
#include "drum_modes.h"
#include "general.h"
#include "accel_control.h"


int main(int argCount, char* args[]) {

	DrumMode_init();
	JoystickControl_init();
	UdpListener_init();
	AccelControl_init();

	// int curVol = 0;

	// char message[COMMAND_LENGTH];
	//
	// struct sockaddr_in sin;
	// memset(&sin, 0, sizeof(sin));
	// sin.sin_family = AF_INET;                   // Connection may be from network
	// sin.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network long
	// sin.sin_port = htons(PORT);                 // Host to Network short
	//
	// // Create the socket for UDP
	// int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
	//
	// // Bind the socket to the port (PORT) that we specify
	// bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

	while (!General_stoppingProgram()) {
		// int tmpVol = AudioMixer_getVolume();
		// if (curVol != tmpVol) {
		// 	// unsigned int sin_len = sizeof(sin);
		// 	curVol = tmpVol;
		// 	sentVolToUDP();
		// 	// sprintf(message, "vol = %d \n", curVol);
		// 	// sendto( socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
		// }
		sleep_usec(SLEEP_TIME_US);
	}

	// close(socketDescriptor);

	AccelControl_cleanup();
	UdpListener_cheanup();
	JoystickControl_cleanup();
	DrumMode_Cleanup();

	return 0;
}

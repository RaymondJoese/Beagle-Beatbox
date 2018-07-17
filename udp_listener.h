// UDP_listener.h
// Module to listen message through UDP port 12345 from Host(client)
#ifndef _UDP_LISTENER_H_
#define _UDP_LISTENER_H_

#define PORT 12345
#define COMMAND_LENGTH 1024
#define REPLY_BUFFER_SIZE 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)

// int tokenizeCommand(char *buff, char *tokens[]);
// int readCommand(char *buff, char *tokens[]);
// void* UDP_listenerThread(void * arg);

void UdpListener_init(void);
void UdpListener_cheanup(void);


#endif

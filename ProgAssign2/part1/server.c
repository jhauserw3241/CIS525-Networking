/* ----------------------------------------------------------------------------------------------*/
/* server.c - sample time/date server.*/
/*-----------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include "inet.h"

#define MSG_MAX 1000
#define NAME_MAX 20
#define PIPE_NUM 2
#define CLI_MAX 100

int main(int argc, char **argv)
{
	int sockfd, newsockfd, clilen, childpid, nnames;
	struct sockaddr_in cli_addr, serv_addr;
	char msg[MSG_MAX];
	char request;
	int num;
	int ret_val;
	fd_set master_fd_set, working_fd_set;
	char names[CLI_MAX][NAME_MAX];

	/* Initialize the list of active sockets */
	FD_ZERO(&master_fd_set);
	FD_ZERO(&working_fd_set);

	/* Create communication endpoint */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("server: can't open stream socket");
		return(1);
	}

	/* Bind socket to local address */
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family 			= AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port 				= htons(SERV_TCP_PORT);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("server: can't bind local address");
		return(1);
	}

	// Listen for connection from client
	listen(sockfd, 5);
	FD_SET(sockfd, &master_fd_set);

	for(;;)
	{
		working_fd_set = master_fd_set;

		// Wait for client to send message
		if(select(FD_SETSIZE, &working_fd_set, NULL, NULL, NULL) < 0) {
			perror("Failure when doing select");
			exit(EXIT_FAILURE);
		}

		// Handle when a socket has something to do
		for(int i = 0; i < FD_SETSIZE; ++i) {
			memset(msg, 0, MSG_MAX);
			if(FD_ISSET(i, &working_fd_set)) {
				// Handle new client wanting to connect
				if(i == sockfd) {
					/* Accept a new connection request. */
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					if(newsockfd < 0) {
						perror("server: accept error");
						return(1);
					}

					/* Add the file descriptor to the list of file descriptors */
					FD_SET(newsockfd, &master_fd_set);
				}
				// Handle client sending message
				else {
					recv(i, msg, MSG_MAX, 0);

					// Approve client name
					if(strstr(msg, "Name: "))
					{
						bool nameAlreadyUsed = false;

						// Parse out name
						char name[NAME_MAX];
						memcpy(name, &msg[6], NAME_MAX);
						for(int v = 0; v < NAME_MAX; v++) {
							if(name[i] == '\n') {
								memset(&name[i], 0, 1);
							}
						}

						// Check if the name is already used
						for(int q = 0; q < nnames; q++) {
							if(strncmp(name, names[q], NAME_MAX) == 0)
							{
								nameAlreadyUsed = true;
							}
						}

						// Handle whether or not the name is already used
						if(nameAlreadyUsed) {
							// Send verification to client
							send(i, "bad", MSG_MAX, 0);
						}
						else {
							// Send verification to client
							if(nnames == 0) {
								send(i, "good; You are the first user to join the chat.", MSG_MAX, 0);
							}
							else {
								send(i, "good", MSG_MAX, 0);
							}

							// Add name to list of names
							strcpy(names[nnames], name);
							nnames++;

							// Tell all current users about new user
							for(int w = 0; w < FD_SETSIZE; ++w) {
								if((w != i) && (w != sockfd)) {
									char newClientAnnouncement[MSG_MAX];
									memset(newClientAnnouncement, 0, MSG_MAX);
									snprintf(newClientAnnouncement, MSG_MAX, "%s has joined the chat.\n", name);
									send(w, newClientAnnouncement, MSG_MAX, 0);
								}
							}
						}

						continue;
					}

					// Remove client
					if(strstr(msg, "quit"))
					{
						close(i);

						// Remove the fd from the master set
						FD_CLR(i, &master_fd_set);
					}

					// Forward message
					for(int j = 0; j < FD_SETSIZE; ++j) {
						if((j != i) && (j != sockfd)) {
							send(j, msg, MSG_MAX, 0);
						}
					}
				}	
			}
		}
	}
}

/*----------------------------------------------------------*/
/* client.c - sample time/date client.                      */
/*----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include "inet.h"

#define NAME_MAX 20
#define MSG_MAX 1000

void exit_shell(int signum);

int									sockfd;
char name[NAME_MAX];

int main(int args, char **argv)
{
	struct sockaddr_in serv_addr;
	char msg[MSG_MAX];
	int interact_status;
	int nread; /* number of characters */
	fd_set master_fd_set, working_fd_set;

	signal(SIGINT, exit_shell);

	/* Initialize lists of sockets */
	FD_ZERO(&master_fd_set);
	FD_ZERO(&working_fd_set);

	/* Set up the address of the server to be contacted. */
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family			= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port				= htons(SERV_TCP_PORT);

	/* Create a socket (an endpoint for communication). */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	  perror("client: can't open stream socket");
		return(1);
	}

	/* Connect to the server. */
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  perror("client: can't connect to the server");
		return(1);
	}

	/* Add file descriptors */
	FD_SET(STDIN_FILENO, &master_fd_set);
	FD_SET(sockfd, &master_fd_set);

	printf("You are now connected to the server.\n");

	bool nameApproved = false;
	while(nameApproved != true) {
		// Get name from user
		printf("Your name: ");
	  fgets(name, NAME_MAX, stdin);
		for(int i = 0; i < NAME_MAX; i++) {
			if(name[i] == '\n') {
				memset(&name[i], 0, 1);
			}
		}

		// Disallow a name that would confuse the server
		if(strcmp(name, "Name") == 0) {
			printf("That name is not allowed. Please choose a different name.\n");
			continue;
		}

		// Prep the message to be sent to the server
		char name_msg[6 + NAME_MAX];
		memset(name_msg, 0, 6 + NAME_MAX);
		snprintf(name_msg, 6 + NAME_MAX, "Name: %s\n", name);

		/* Send the user's name to the server. */
		write(sockfd, name_msg, 6 + NAME_MAX);

		memset(msg, 0, MSG_MAX);
		nread = read(sockfd, msg, MSG_MAX);
		if(nread <= 0) {
			printf("Nothing read, nread = %d.\n", nread);
		}

		if(strstr(msg, "good")) {
			nameApproved = true;

			if(strstr(msg, "good; ")) {
				char firstPersonMsg[MSG_MAX];
				memcpy(firstPersonMsg, &msg[6], MSG_MAX);
				printf("%s\n", firstPersonMsg);
			}
		}
		else {
			printf("That name is already taken. Please choose a different name.\n");
		}
	}

	/* Allow the user to chat with other users. */
	for(;;) {
		working_fd_set = master_fd_set;

		// Wait for activity on file descriptor
		if(select(FD_SETSIZE, &working_fd_set, NULL, NULL, NULL) < 0) {
			perror("select: failure");
			exit(EXIT_FAILURE);
		}

		// Handle when a file descriptor has something to do
		for(int i = 0; i < FD_SETSIZE; ++i) {
			if(FD_ISSET(i, &working_fd_set)) {
				// Hanlde input from user
				if(i == STDIN_FILENO) {
					// Get the message from the user
					memset(msg, 0, MSG_MAX);
					fgets(msg, MSG_MAX, stdin);
					msg[strcspn(msg, "\n")] = 0;

					// Don't allow message that cuts off connection to server
					if(strcmp(msg, "quit") == 0)
					{
						printf("That message is not allowed\n");
					}
					// Send message to server
					else {
						// Prep the message to be sent to the server
						char temp[NAME_MAX + 2 + MSG_MAX];
						memset(temp, 0, NAME_MAX + 2 + MSG_MAX);
						snprintf(temp, NAME_MAX + 2 + MSG_MAX, "%s: %s\n", name, msg);

						/* Send the user's response to the server. */
						write(sockfd, temp, MSG_MAX);
					}
				}
				// Handle message from server
				else {
					nread = read(sockfd, msg, MSG_MAX);
					if(nread > 0) {
						printf("%s", msg);
					} else {
						printf("Nothing read, nread = %d.\n", nread);
					}			
				}
			}
		}

	}
	
	close(sockfd);

	return(0);	/* Exit if response is 4. */
}

void exit_shell(int signum) {
	char temp[NAME_MAX + 6];
	memset(temp, 0, NAME_MAX + 6);
	snprintf(temp, NAME_MAX + 6, "%s: quit", name);
	write(sockfd, temp, MSG_MAX);
	close(sockfd);
	exit(1);
}

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
#include "shared.c"

#define NAME_MAX 20
#define MSG_MAX 1000

void exit_shell(int signum);

int serv_fd;
char name[NAME_MAX];

int main(int args, char **argv)
{
	int dir_fd, serv_fd;
	struct sockaddr_in dir_addr, serv_addr;
	char msg[MSG_MAX], serv_name[NAME_MAX], serv_info[MSG_MAX];
	int nread; /* number of characters */
	fd_set master_fd_set, working_fd_set;
	uint16_t serv_port;

	signal(SIGINT, exit_shell);

	/* Initialize lists of sockets */
	FD_ZERO(&master_fd_set);
	FD_ZERO(&working_fd_set);

	printf("Start setup of chat server\n");

	/* Set up the address of the server to be contacted. */
	memset((char *) &dir_addr, 0, sizeof(dir_addr));
	dir_addr.sin_family				= AF_INET;
	dir_addr.sin_addr.s_addr	= inet_addr(DIR_HOST_ADDR);
	dir_addr.sin_port					= htons(DIR_TCP_PORT);

	printf("Before creating socket\n");
	/* Create a socket (an endpoint for communication). */
	if((dir_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	  perror("client: can't open stream socket to directory server");
		return(1);
	}

	printf("Before connection to directory server\n");
	/* Connect to the server. */
	if(connect(dir_fd, (struct sockaddr *) &dir_addr, sizeof(dir_addr)) < 0) {
	  perror("client: can't connect to the directory server");
		return(1);
	}

	printf("After connecting to dir server\n");

	/* Add file descriptors */
	FD_SET(STDIN_FILENO, &master_fd_set);

	printf("You are now connected to the directory server.\n");

	char test[MSG_MAX];
	memset(test, 0, MSG_MAX);
	snprintf(test, MSG_MAX, "Get directory");

	/* Send the user's name to the server. */
	write(dir_fd, test, MSG_MAX);

	printf("Send message to chat dir server\n");

	memset(msg, 0, MSG_MAX);
	nread = read(dir_fd, msg, MSG_MAX);
	if(nread <= 0) {
		printf("Nothing read, nread = %d.\n", nread);
	}
	printf("%s\n", msg);

	memset(serv_info, 0, MSG_MAX);

	// Get info from user
	printf("Select server: ");
	fgets(serv_info, MSG_MAX, stdin);
	for(int i = 0; i < MSG_MAX; i++) {
		if(serv_info[i] == '\n') {
			memset(&serv_info[i], 0, 1);
		}
	}

	printf("Server name: %s\n", serv_info);

	// Get server port
	serv_port = get_port_from_serv_info(serv_info);

	printf("Port #: %d\n", serv_port);

	close(dir_fd);

	/* Set up the address of the server to be contacted. */
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family				= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port					= htons(serv_port);

	/* Create a socket (an endpoint for communication). */
	if((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	  perror("client: can't open stream socket to chat server");
		return(1);
	}

	/* Connect to the server. */
	if(connect(serv_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  perror("client: can't connect to the chat server");
		return(1);
	}

	/* Add file descriptors */
	FD_SET(serv_fd, &master_fd_set);

	printf("You are now connected to the chat server.\n");

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
		write(serv_fd, name_msg, 6 + NAME_MAX);

		memset(msg, 0, MSG_MAX);
		nread = read(serv_fd, msg, MSG_MAX);
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
						write(serv_fd, temp, MSG_MAX);
					}
				}
				// Handle message from server
				else {
					nread = read(serv_fd, msg, MSG_MAX);
					if(nread > 0) {
						printf("%s", msg);
					} else {
						printf("Nothing read, nread = %d.\n", nread);
					}			
				}
			}
		}

	}
	
	close(serv_fd);

	return(0);	/* Exit if response is 4. */
}

void exit_shell(int signum) {
	char temp[NAME_MAX + 6];
	memset(temp, 0, NAME_MAX + 6);
	snprintf(temp, NAME_MAX + 6, "%s: quit", name);
	write(serv_fd, temp, MSG_MAX);
	close(serv_fd);
	exit(1);
}

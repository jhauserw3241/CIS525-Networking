/* ----------------------------------------------------------------------------------------------*/
/* server.c - multi-client chat server.*/
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
#include "shared.c"

int dir_fd;

void exit_shell(int signum);

int main(int argc, char *argv[])
{
	int listen_fd, new_fd, clilen, childpid, nnames;
	struct sockaddr_in cli_addr, serv_addr, dir_addr;
	char msg[MSG_MAX];
	char request;
	int num;
	int ret_val;
	fd_set master_fd_set, working_fd_set;
	char names[CLI_MAX][NAME_MAX];
	char server_name[NAME_MAX];
	uint16_t port;

	signal(SIGINT, exit_shell);

	// Verify that the correct num of args were given
	if(argc != 3) {
		printf("Incorrect number of arguments.\n");
		printf("You need to structure the command as seen below:\n");
		printf("    ./server 'Chat server name' <port num>\n");
		exit(1);
	}

	// Get server name
	if(strlen(argv[1]) <= NAME_MAX) {
		memset(server_name, 0, NAME_MAX);
		snprintf(server_name, NAME_MAX, "%s", argv[1]);
	}
	else {
		printf("The server name must be less than 20 characters\n");
		exit(1);
	}

	if(strstr(server_name, ";")) {
		printf("The server name cannot contain a semicolon\n");
		exit(1);
	}

	if(strstr(server_name, ",")) {
		printf("The server name cannot contain a comma\n");
		exit(1);
	}

	// Get port num
	port = str_to_int(argv[2]);

	/* Initialize the list of active sockets */
	FD_ZERO(&master_fd_set);
	FD_ZERO(&working_fd_set);

	/* Create communication endpoint */
	if((dir_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("server: can't open stream socket to directory");
		return(1);
	}

	/* Connect to the directory server */
	memset((char *) &dir_addr, 0, sizeof(dir_addr));
	dir_addr.sin_family 			= AF_INET;
	dir_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	dir_addr.sin_port 				= htons(DIR_TCP_PORT);

	if(connect(dir_fd, (struct sockaddr *) &dir_addr, sizeof(dir_addr)) < 0) {
		perror("server: can't connect to directory");
		return(1);
	}

	bool regRecvd = false;
	while(regRecvd == false) {
		char regMsg[MSG_MAX];
		memset(regMsg, 0, MSG_MAX);
		snprintf(regMsg, MSG_MAX, "Register server:%s;%d", server_name, port);

		// Send registration to directory server
		send(dir_fd, regMsg, MSG_MAX, 0);

		// Get confirmation of registration
		memset(msg, 0, MSG_MAX);
		read(dir_fd, msg, MSG_MAX);

		if(!strcmp(msg, "Registered")) {
			regRecvd = true;
		}
		else {
			perror("Could not register with directory server");
			return(1);
		}
	}

	/* Create communication endpoint */
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("server: can't open stream socket");
		return(1);
	}

	/* Bind socket to local address */
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family 			= AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port 				= htons(port);

	if(bind(listen_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("server: can't bind local address");
		return(1);
	}

	// Listen for connection from client
	listen(listen_fd, 5);
	FD_SET(listen_fd, &master_fd_set);

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
				if(i == listen_fd) {
					/* Accept a new connection request. */
					clilen = sizeof(cli_addr);
					new_fd = accept(listen_fd, (struct sockaddr *) &cli_addr, &clilen);
					if(new_fd < 0) {
						perror("server: accept error");
						return(1);
					}

					/* Add the file descriptor to the list of file descriptors */
					FD_SET(new_fd, &master_fd_set);
				}
				// Handle client sending message
				else {
					recv(i, msg, MSG_MAX, 0);
					printf("Client msg: %s\n", msg);

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
								if((w != i) && (w != listen_fd) && (w != dir_fd)) {
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
						if((j != i) && (j != listen_fd) && (j != dir_fd)) {
							send(j, msg, MSG_MAX, 0);
						}
					}
				}	
			}
		}
	}
}

void exit_shell(int signum) {
	char temp[MSG_MAX];
	memset(temp, 0, MSG_MAX);
	snprintf(temp, MSG_MAX, "De-register server");
	write(dir_fd, temp, MSG_MAX);
	close(dir_fd);
	exit(1);
}

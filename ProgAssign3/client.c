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
#include <netdb.h>
#include "inet.h"

#define IP_ADDR_MAX 50
#define MSG_MAX 1000

void exit_shell(int signum);
int str_to_int(const char *s);

int	sockfd;

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
	char msg[MSG_MAX];
	int interact_status;
	int nread; /* number of characters */
	fd_set master_fd_set, working_fd_set;
	char serv_ip[IP_ADDR_MAX];
	int serv_port = 80;

	// Verify the correct number of arguments were provided
	if((argc < 2) || (argc > 3))
	{
		perror("There should only be two arguments: the website name and the port number (optional).");
		return(1);
	}

	signal(SIGINT, exit_shell);

	// Initialize lists of sockets
	FD_ZERO(&master_fd_set);
	FD_ZERO(&working_fd_set);

	// Get the website name
	printf("Hello world!\n");
	printf("The website to access: %s\n", argv[1]);

	// Get the website ip address
	struct hostent *hp = gethostbyname(argv[1]);

	if(hp == NULL)
	{
		perror("gethostbyname() failed");
		return(1);
	}

	printf("%s = ", hp->h_name);
	unsigned int i = 0;
	while(hp->h_addr_list[i] != NULL)
	{
		printf("%s ", inet_ntoa(*(struct in_addr*)(hp->h_addr_list[i])));
		i++;
	}
	printf("\n");

	memset(serv_ip, 0, IP_ADDR_MAX);
	snprintf(serv_ip, IP_ADDR_MAX, "%s", inet_ntoa(*(struct in_addr*)(hp->h_addr_list[0])));
	printf("IP address: %s\n", serv_ip);

	// Get the website port number
	if(argc == 3)
	{
		serv_port = str_to_int(argv[2]);
	}

	printf("The server port to connect to is: %d\n", serv_port);

	/*// Set up the address of the server to be contacted.
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family			= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port				= htons(SERV_TCP_PORT);

	// Create a socket (an endpoint for communication).
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	  perror("client: can't open stream socket");
		return(1);
	}

	// Connect to the server.
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  perror("client: can't connect to the server");
		return(1);
	}

	// Add file descriptors
	FD_SET(STDIN_FILENO, &master_fd_set);
	FD_SET(sockfd, &master_fd_set);

	printf("You are now connected to the server.\n");

	// Allow the user to chat with other users.
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

					// Prep the message to be sent to the server
					char temp[NAME_MAX + 2 + MSG_MAX];
					memset(temp, 0, NAME_MAX + 2 + MSG_MAX);
					snprintf(temp, NAME_MAX + 2 + MSG_MAX, "%s: %s\n", name, msg);

					// Send the user's response to the server.
					write(sockfd, temp, MSG_MAX);
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
	
	close(sockfd); */

	return(0);	// Exit if response is 4.
}

int str_to_int(const char *s)
{
	int res = 0;
	while(*s)
	{
		res *= 10;
		res += *s++ - '0';
	}

	return res;
}

void exit_shell(int signum) {
	close(sockfd);
	exit(1);
}

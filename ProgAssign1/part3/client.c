/*----------------------------------------------------------*/
/* client.c - sample time/date client.                      */
/*----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "inet.h"

#define MAX 100

int interact_with_server(void);
void exit_shell(int signo);
void get_response(void);
int readn(int, char *, int);

int									sockfd;
struct sockaddr_in	serv_addr;
char								response[MAX]; /* user response */

int main(int args, char **argv)
{
	int interact_status;

	signal(SIGINT, exit_shell);

	/* Set up the address of the server to be contacted. */
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family			= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port				= htons(SERV_TCP_PORT);

	get_response();

	/* Display the menu, read user's response, and send it to the server. */
	while(strcmp(response, "4")) {
		if((strcmp(response, "1") != 0) &&
			 (strcmp(response, "2") != 0) &&
			 (strcmp(response, "3") != 0)) {
			sprintf(response, "5");
		}

		interact_status = interact_with_server();
		if(interact_status != 0) {
			return(interact_status);
		}

		get_response();
	}

	return(0);	/* Exit if response is 4. */
}

void exit_shell(int signo)
{
	sprintf(response, "0");
	exit(interact_with_server());
}

int interact_with_server()
{
	int		nread;	/* number of characters */
	char	s[MAX]; /* array to hold output */

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

	sprintf(s, "%s\n", response);

	/* Send the user's response to the server. */
	write(sockfd, s, 1);

	/* Read the server's response */
	nread = readn(sockfd, s, MAX);
	if(nread > 0) {
		printf("    %s\n", s);
	} else {
		printf("Nothing read, nread = %d.\n", nread);
	}

	close(sockfd);

	return(0);
}

/* Display menu and retrieve user's response */
void get_response()
{
	printf("==================================================\n");
	printf("                  Menu:\n");
	printf("--------------------------------------------------\n");
	printf("                 1. Time\n");
	printf("          2. PID of the server\n");
	printf("  3. Random number between 11 and 30, inclusive\n");
	printf("                 4. Quit\n");
	printf("--------------------------------------------------\n");
	printf("                 Choice (1-4):\n");
	scanf("%s", response);
	printf("==================================================\n");
}

/*
 * Read up to "n" bytes from a descriptor.
 * Use in place of read() when fd is a stream socket.
 */
int readn(int fd, char* ptr, int nbytes)
{
	int nleft, nread;

	nleft = nbytes;
	while(nleft > 0) {
		nread = read(fd, ptr, nleft);

		if(nread < 0) {
			return(nread); /* error, return < 0 */
		} else if (nread == 0) {
			break; /* EOF */
		}

		nleft -= nread;
		ptr += nread;
	}

	return (nbytes - nleft); /* return >= 0 */
}

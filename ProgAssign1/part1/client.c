/*----------------------------------------------------------*/
/* client.c - sample time/date client.                      */
/*----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "inet.h"

#define MAX 100

void get_response(void);

char response[MAX];		/* user response */

int main(int args, char **argv)
{
	int 								sockfd;
	struct sockaddr_in 	cli_addr, serv_addr;
	char 								s[MAX];					/* array to hold output */
	int									nread;					/* number of characters */
	int									servlen;				/* length of server addr */
	char								request;

	/* Set up the address of the server to be contacted. */
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family			= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port				= htons(SERV_UDP_PORT);

	/* Set up the address of the client. */
	memset((char *) &cli_addr, 0, sizeof(cli_addr));
	cli_addr.sin_family				= AF_INET;
	cli_addr.sin_addr.s_addr	= htonl(0);
	cli_addr.sin_port					= htons(0);

	/* Create a socket (an endpoint for communication). */
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("client: can't open datagram socket");
		exit(1);
	}

	/* Bind the client's socket to the client's address. */
	if(bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
		perror("client: can't bind local address");
		exit(1);
	}

	printf("%s \n", inet_ntoa(cli_addr.sin_addr));

	get_response();

	/* Display the menu, read user's response, and send it to the server. */
	while(strcmp(response, "4")) {
		if((strcmp(response, "1") != 0) &&
			 (strcmp(response, "2") != 0) &&
			 (strcmp(response, "3") != 0)) {
			sprintf(response, "5");
		}

		/* Send the user's response to the server. */
		servlen = sizeof(serv_addr);
		request = (char)response[0];

		sendto(sockfd, (char *) &request, sizeof(request), 0, (struct sockaddr *) &serv_addr, servlen);

		/* Read the server's response. */
		nread = recvfrom(sockfd, s, MAX, 0, (struct sockaddr *) &serv_addr, &servlen);
		if(nread > 0) {
			printf("    %s\n", s);
		} else {
			printf("Nothing read.\n");
		}

		get_response();
	}

	exit(0);	/* Exit if response is 4. */
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

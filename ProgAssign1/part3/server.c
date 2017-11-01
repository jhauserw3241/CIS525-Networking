/* ----------------------------------------------------------------------------------------------*/
/* server.c - sample time/date server.*/
/*-----------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "inet.h"

#define MAX 100

int main(int argc, char **argv)
{
	int									sockfd, newsockfd, clilen, childpid;
	struct sockaddr_in	cli_addr, serv_addr;
	struct tm						*timeptr; /* pointer to time structure */
	time_t							clock; /* clock value (in secs) */
	char								s[MAX];
	char								request;
	int 								ret_val;
	int 								num;

	/* Parent should ignore the death of a child. */
	signal(SIGCHLD, SIG_IGN);

	/* Create communication endpoint */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("server: can't open stream socket");
		return(1);
	}

	/* Bind socket to local address */
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family 			= AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port 				= htons(SERV_TCP_PORT);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("server: can't bind local address");
		return(1);
	}

	listen(sockfd, 5);

	for(;;)
	{
		/* Accept a new connection request. */
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd < 0) {
			perror("server: accept error");
			return(1);
		}

		/* Create a slave to do the work. */
		ret_val = fork();
		if(ret_val < 0) {
			perror("server: unable to fork");
			return(2);
		}

		if(ret_val == 0) {
			/* Read the request from the client. */
			close(sockfd); /* The slave does not use sockfd */
			read(newsockfd, &request, sizeof(char));
			printf("Received: %c\n", request);

			/* Generate an appropriate reply. */
			clock = time(0);
			timeptr = localtime(&clock);

			switch(request)
			{
				case '0':	sprintf(s, "Goodbye!");
									break;
				case '1': strftime(s, MAX, "%T", timeptr);
									break;
				case '2': sprintf(s, "%d", getpid());
									break;
				case '3': num = (rand() % (30 + 1));
									sprintf(s, "%d", num);
									break;
				default: 	strcpy(s, "Invalid request\n");
									break;
			}

			/* Send the reply to the client. */
			write(newsockfd, s, strlen(s) + 1);
			close(newsockfd);
			return(0); /* The slave terminates. */
		}

		close(newsockfd); /* The parent does not use newsockfd. */
	}
}

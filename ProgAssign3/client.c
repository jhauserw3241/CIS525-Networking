/*----------------------------------------------------------*/
/* client.c - sample time/date client.                      */
/*----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <netdb.h>
#include <ctype.h>
#include "inet.h"

#define IP_ADDR_MAX 50
#define MSG_MAX 1000

void exit_shell(int signum);
int str_to_int(const char *s);

int	serv_fd;

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
	char msg[MSG_MAX];
	int interact_status;
	int nread; /* number of characters */
	char serv_ip[IP_ADDR_MAX];
	int serv_port = 80;
	char page[MSG_MAX];

	// Verify the correct number of arguments were provided
	if((argc < 2) || (argc > 3))
	{
		perror("There should only be two arguments: the website name and the port number (optional).");
		return(1);
	}

	signal(SIGINT, exit_shell);

	// Get the website ip address
	struct hostent *hp = gethostbyname(argv[1]);
	if(hp == NULL)
	{
		perror("gethostbyname() failed");
		return(1);
	}

	unsigned int i = 0;
	while(hp->h_addr_list[i] != NULL)
	{
		i++;
	}

	memset(serv_ip, 0, IP_ADDR_MAX);
	snprintf(serv_ip, IP_ADDR_MAX, "%s", inet_ntoa(*(struct in_addr*)(hp->h_addr_list[0])));

	// Get the website port number
	if(argc == 3)
	{
		serv_port = str_to_int(argv[2]);
		if(serv_port < 0)
		{
			perror("Invalid port number");
			return(1);
		}
	}

	printf("Trying %s on port %d...\n", serv_ip, serv_port);

	// Get the page to search for one the web server
	printf("Which page? (e.g. '/~eyv/')\n");
	memset(page, 0, MSG_MAX);
	fgets(page, MSG_MAX, stdin);
	for(int i = 0; i < MSG_MAX; i++)
	{
		if(page[i] == '\n')
		{
			memset(&page[i], 0, 1);
		}
	}

	// Set up the address of the server to be contacted.
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family			= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr(serv_ip);
	serv_addr.sin_port				= htons(serv_port);

	// Create a socket (an endpoint for communication).
	if((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	  perror("client: can't open stream socket");
		return(1);
	}

	// Connect to the server.
	if(connect(serv_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  perror("client: can't connect to the server");
		return(1);
	}

	printf("Connected to %s.\n", hp->h_name);

	char request[MSG_MAX + 4];
	memset(request, 0, MSG_MAX + 4);
	snprintf(request, MSG_MAX + 4, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", page, argv[1]);
	write(serv_fd, request, MSG_MAX);
	printf("The request is: %s\n", request);

	printf("--------------------------START-------------------------\n");

	// Handle message from server
	int content_size;
	char *content = malloc(content_size);

	char temp[MSG_MAX];
	memset(temp, 0, MSG_MAX);
	while(read(serv_fd, temp, MSG_MAX) != 0)
	{
		content_size = content_size + MSG_MAX;
		char *content_switch = realloc(content, content_size);
		strcpy(content_switch, content);
		strcat(content_switch, temp);

		memset(temp, 0, MSG_MAX);
	}
	printf("Content: %s\n", content);

	// Check status line
	char status[MSG_MAX];
	memset(status, 0, MSG_MAX);
	int statusSize = -1;
	for(int j = 0; j < MSG_MAX; j++)
	{
		if(content[j] == '\n')
		{
			statusSize = j;
			break;
		}
	}
	snprintf(status, statusSize, "%s", content);
	
	// Handle error status
	if((strcmp(status, "HTTP/1.0 200 OK") != 0) &&
		 (strcmp(status, "HTTP/1.1 200 OK") != 0))
	{
		printf("%s\n", status);
	}
	// Handle OK status
	else {
		printf("Add code to print out the page here\n");
	}

	printf("---------------------------END--------------------------\n");
	
	close(serv_fd);

	return(0);	// Exit if response is 4.
}

int str_to_int(const char *s)
{
	for(int i = 0; i < strlen(s); i++)
	{
		if(!isdigit(s[i]))
		{
			return -1;
		}
	}

	int res = 0;
	while(*s)
	{
		res *= 10;
		res += *s++ - '0';
	}

	return res;
}

void exit_shell(int signum) {
	close(serv_fd);
	exit(1);
}

/*
 * Definitions for TCP and UDP client/server programs.
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERV_HOST_ADDR "127.0.0.1" /* Change this to by your host addr */
#define DIR_TCP_PORT 8126
#define DIR_HOST_ADDR "127.0.0.1"

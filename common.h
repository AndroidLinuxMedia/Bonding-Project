#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>
#include<ifaddrs.h>
#include<sys/ioctl.h>
#include<net/if.h>

#define VERSION  0

#define MAX_MESSAGE_LENGHT 2000
#define MAX_CONNECTION_SUPPORT 10

#define CMD_VERSION_NUMBER      0
#define CMD_LIST_OF_INTERFACE   1
#define CMD_CONNECT             2
#define CMD_GET_DATA		3

typedef struct _packets {
	char version;
	long long packet_count;
	long long timestamp;
	char command;
	char data[MAX_CONNECTION_SUPPORT][20];
	int data_length;
}packets;

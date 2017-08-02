/*
 * Using socket programming this program send data packets to receiver
 * using all available interfaces.
 */

#include "common.h"

char address[MAX_CONNECTION_SUPPORT][20];
int  interface_count;

int get_list_of_interface( void );
int create_socket( char *interface_ip );

int main( int argc , char *argv[] )
{
	int count		= 0;
	int interface 		= 0;
	int sock[MAX_CONNECTION_SUPPORT];
	int nsock		= 0;
	int new_sock		= 0;
	packets data;
	int result		= 0;
	struct sockaddr_in si_server;

	if (argc != 2) {
		printf("Invalid argument received\n");
		printf("Example: %s <receiver ip>\n", argv[0]);
		return -1;
	}
	interface = get_list_of_interface();

	if ((interface == 0) || (interface > MAX_CONNECTION_SUPPORT)) {
		printf("Number of interfaces are out of range: %d\n", interface);
		return 0;
	}
	for(count = 0; count < interface; count++) {
		new_sock = create_socket( address[count] );
		if (new_sock > 0) {
			sock[nsock++] = new_sock;
		}
	}

	if (nsock == 0) {
		printf("Failed to connect receiver ip: %s\n", argv[1]);
		return -1;
	}

	si_server.sin_family = AF_INET;
	si_server.sin_port = htons(8888);
	printf("ip address %s\n", argv[1]);
	if (inet_aton(argv[1] , &si_server.sin_addr) == 0)
	{
		fprintf(stderr, "inet_aton() failed\n");
		return -1;
	}

	memset(&data, 0x00, sizeof(packets));
	printf("sending data with %d interface\n", nsock);
	while(1)
	{
		for(count = 0; count < nsock; count++) {
			data.command = CMD_VERSION_NUMBER;
			data.packet_count++;
			data.data_length = 512;
#if 0
			result = sendto( sock[count], (char *)&data, sizeof(packets), 0, (struct sockaddr *) &si_server, sizeof(si_server) );
			if (result < 0)
			{
				printf("Failed to send packet from interface number : %d\n", count);
				continue;
			}
#endif
			data.data = (char*)malloc(data.data_length);
			if (data.data == NULL) {
				printf("Failed to allocate memory\n");
				break;
			}
			printf("send to: %s\n", inet_ntoa(si_server.sin_addr));
			sprintf( data.data, "Package count %lld and data length %d", data.packet_count, data.data_length);
			result = sendto( sock[count], data.data, data.data_length, 0, (struct sockaddr *) &si_server, sizeof(si_server) );
			if (result < 0)
			{
				printf("Failed to send data");
				free(data.data);
				continue;
			}
			free(data.data);
		}
		usleep(100000);
	}

	for(count = 0; count < nsock; count++) {
		close(sock[count]);
	}
	return 0;
}

int create_socket( char *interface_ip )
{
	int socket_desc		= 0;
	char *recv_ip		= NULL;
	char *intf_ip		= NULL;
	struct sockaddr_in server;

	if ( interface_ip == NULL ) {
		printf("Invalid argument received\n");
		return -1;
	}

	intf_ip = (char *)interface_ip;

	socket_desc = socket( AF_INET , SOCK_DGRAM, 0);
	if ( socket_desc == -1 ) {
		printf("Failed to create socket\n");
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr( intf_ip );
	server.sin_port = htons( 8888 );
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("bind");
		return -1;
	}
	return socket_desc;
}
long long current_timestamp() {
	struct timeval te;
	gettimeofday(&te, NULL);
	long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
	return milliseconds;
}
/* This function will send program version number to client
 * for further communincations.
 */
void get_version(int sock, packets *data)
{
	data->packet_count += 2;
	data->version = VERSION;
	data->timestamp = current_timestamp();
}

int get_list_of_interface( void )
{
	struct ifaddrs *ifaddr, *ifa;
	getifaddrs(&ifaddr);
	int i;
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if ((ifa->ifa_addr != NULL) && ifa->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *pAddr = (struct sockaddr_in *)ifa->ifa_addr;
			struct ifreq ifr;
			int sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_IP);
			memset(&ifr, 0, sizeof(ifr));
			strcpy(ifr.ifr_name,  ifa->ifa_name);
			if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
				perror("SIOCGIFFLAGS");
			}
			if ( (ifr.ifr_flags & IFF_UP) &&
			     !(ifr.ifr_flags  & IFF_LOOPBACK) &&
			     (ifr.ifr_flags & IFF_RUNNING) &&
			     !(ifr.ifr_flags & IFF_POINTOPOINT)) {
				printf("name %s : %s\n", ifa->ifa_name, inet_ntoa(pAddr->sin_addr));
				strcpy(address[interface_count++], inet_ntoa(pAddr->sin_addr));
			}
			close(sock);
		}
	}
	return interface_count;
}

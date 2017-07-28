/*
 * Using socket programming this program send data packets to receiver
 * using all available interfaces.
 */

#include "common.h"

char address[MAX_CONNECTION_SUPPORT][20];
int  interface_count;

void *client_connection_handler( void * );
int get_list_of_interface( void );
int create_socket( char *receiver_ip, char *interface_ip );

int main( int argc , char *argv[] )
{
	int count		= 0;
	int interface 		= 0;
	int sock[MAX_CONNECTION_SUPPORT];
	int nsock		= 0;
	int new_sock		= 0;
	packets data;
	int result		= 0;

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
		new_sock = create_socket( argv[1], address[count]);
		if (new_sock > 0) {
			sock[nsock++] = new_sock;
		}
	}

	if (nsock == 0) {
		printf("Failed to connect receiver ip: %s\n", argv[1]);
		return -1;
	}
	memset(&data, 0x00, sizeof(packets));
	printf("sending data with %d interface\n", nsock);
	while(result >= 0)
	{
		for(count = 0; count < nsock; count++) {
			data.command = CMD_VERSION_NUMBER;
			data.packet_count++;
			data.data_length = 30 + data.packet_count;
			result = Send( sock[count], (char *)&data, sizeof(packets) );
			if (result < 0)
			{
				printf("Failed to send packet\n");
				break;
			}
			data.data = (char*)malloc(data.data_length);
			if (data.data == NULL) {
				printf("Failed to allocate memory\n");
				break;
			}
			sprintf( data.data, "Package count %lld and data length %d", data.packet_count, data.data_length);
			result = Send( sock[count], data.data, data.data_length );
			if (result < 0)
			{
				printf("Failed to send data");
				free(data.data);
				break;
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

int create_socket( char *receiver_ip, char *interface_ip)
{
	int socket_desc		= 0;
	char *recv_ip		= NULL;
	char *intf_ip		= NULL;
	struct sockaddr_in server;

	if ((receiver_ip == NULL) || (interface_ip == NULL)) {
		printf("Invalid argument received\n");
		return -1;
	}

	recv_ip = (char *)receiver_ip;
	intf_ip = (char *)interface_ip;

	socket_desc = socket( AF_INET , SOCK_STREAM , 0 );
	if ( socket_desc == -1 ) {
		printf("Failed to create socket\n");
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr( intf_ip );
	server.sin_port = htons( 1234 );
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("bind");
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr( recv_ip );
	server.sin_port = htons( 1234 );

	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0) {
		perror("connect");
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

int Recv (int sock, char *data, int max_size)
{
	int receive_bytes = 0;
	int ret_bytes = 0;

	if ((data == NULL) ||
	    (max_size <= 0))
	{
		return -1;
	}
	do
	{
		ret_bytes = recv (sock, (data + receive_bytes), max_size, MSG_NOSIGNAL);
		if (ret_bytes <= 0)
		{
			printf ("Error while receving data from socket\n");
			receive_bytes = -1;
			break;
		}

		receive_bytes += ret_bytes;
		max_size -= ret_bytes;
	} while (max_size);
	return receive_bytes;
}

int Send (int sock, char *data, int max_size)
{
	int sent_bytes = 0;
	int ret_bytes = 0;

	if ((data == NULL) ||
	    (max_size <= 0))
	{
		return -1;
	}
	do
	{
		ret_bytes = send (sock, (data + sent_bytes), max_size, MSG_NOSIGNAL);
		if (ret_bytes < 0)
		{
			printf ("Error while sending data to socket\n");
			sent_bytes = -1;
			break;
		}
		sent_bytes += ret_bytes;
		max_size -= ret_bytes;
	} while (max_size);
	return sent_bytes;
}


int to_recv_data( void *socket_desc, packets *data)
{
	fd_set master_set, working_set;
	int result = 0;
	struct timeval tv;
	if ((data == NULL) || (socket_desc == NULL)) {
		printf("Invalid argument received, %s", __func__);
		return result;
	}
	int sock = *(int*)socket_desc;

	FD_ZERO(&master_set);
	FD_SET(sock, &master_set);
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	memcpy(&working_set, &master_set, sizeof(master_set));

	result = select( sock + 1, &working_set, NULL, NULL, &tv);
	if ( result == -1 ) {
		printf ( "Failed to receive data exit now\n" );
		return result;
	} else if ( result > 0 && FD_ISSET( sock, &working_set) ) {

		result = Recv(sock , (char *)data, sizeof(packets));
		if (result < 0 )
		{
			printf ( "Failed to receive command\n" );
			return result;
		}
		if (result == 0) {
			printf( "Socket closed\n" );
			return result;
		}
	}
	return result;
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
/*
 * This will handle connection for each client
 * */
void *client_connection_handler(void *socket_desc)
{
	int sock = *(int*)socket_desc;
	int read_size;
	packets data;
	fd_set master_set, working_set;
	int result = 0;
	struct timeval tv;
	int count;

	while( 1 )
	{
		result = to_recv_data(&sock, &data);
		if (result <= 0) {
			break;
		}
		printf ( "Received command: %d\n", data.command );

		switch ( data.command ) {
			case CMD_VERSION_NUMBER:
				get_version( sock, &data );
				write( sock , &data, sizeof(packets) );
				break;
			case CMD_CONNECT:
				//TODO handle connection request
				get_version( sock, &data );
				write( sock , &data, sizeof(packets) );
				break;
			case CMD_GET_DATA:
				//TODO  send data
				get_version( sock, &data );
				write( sock , &data, sizeof(packets) );
				break;
			default:
				printf ( "invalid command %d received\n", data.command );
				get_version( sock, &data );
				write( sock , &data, sizeof(packets) );
		}
	}

	free(socket_desc);

	return 0;
}

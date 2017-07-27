/*
 * Receiver that support to receive data from multiple interface of sender
 */

#include "common.h"

char address[MAX_CONNECTION_SUPPORT][20];
int  interface_count;


int client_number = 0;
void *client_connection_handler(void *socket_desc);

int main(int argc , char *argv[])
{
	int sock;
	struct sockaddr_in server;
	packets data;
	int count;
	struct sockaddr_in client;
	int client_addr_len     = 0;
	int client_sock = 0;
	char *new_client_sock = NULL;
	pthread_t client_thread[MAX_CONNECTION_SUPPORT];

	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) {
		printf( "Could not create socket" );
		return -1;
	}
	server.sin_addr.s_addr = inet_addr( "192.168.1.21" );
	server.sin_family = AF_INET;
	server.sin_port = htons( 1234 );

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
		printf("Failed to enable reusable command\n");
		return -1;
	}

	if( bind(sock,(struct sockaddr *)&server , sizeof(server)) < 0) {
		printf("Failed to bind socket\n");
		return -1;
	}
	listen( sock , 3 );
	client_addr_len = sizeof(struct sockaddr_in);
	while( (client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t*)&client_addr_len)) )
	{
		printf( "New client connection accepted\n" );
		printf( "Client address: %s\n", inet_ntoa(client.sin_addr));
		new_client_sock = malloc( sizeof(char) );
		if ( new_client_sock == NULL) {
			printf( "Failed to allocate memory for new client handle\n" );
			break;
		}
		*new_client_sock = client_sock;

		if( pthread_create( &client_thread[client_number++] , NULL ,  client_connection_handler , (void*) new_client_sock ) < 0)
		{
			printf( "could not create thread\n" );
			break;
		}
		if ( client_number == MAX_CONNECTION_SUPPORT) {
			printf("Maximum client reached : %d\n", client_number);
			break;
		}
	}
	for(count = 0 ; count < client_number; count++) {
		pthread_join(client_thread[count], NULL);
	}
	close(sock);
	return 0;
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

		result = recv(sock , data, sizeof(packets), 0);
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


long long current_timestamp() {
	struct timeval te;
	gettimeofday(&te, NULL);
	long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
	return milliseconds;
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
			printf("Failed to receive data exit thread\n");
			break;
		}
		switch ( data.command ) {
			case CMD_VERSION_NUMBER:
				printf("packet count: %lld\n", data.packet_count);
				break;
			default:
				printf ( "invalid command %d received\n", data.command );
		}
	}

	free(socket_desc);
	return 0;
}

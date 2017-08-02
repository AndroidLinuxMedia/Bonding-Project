/*
 * Receiver that support to receive data from multiple interface of sender
 */

#include "common.h"

char address[MAX_CONNECTION_SUPPORT][20];
int  interface_count;

int client_number = 0;

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
	int result;
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);

	sock = socket(AF_INET , SOCK_DGRAM, 0);
	if (sock == -1) {
		printf( "Could not create socket" );
		return -1;
	}
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons( 8888 );

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
		printf("Failed to enable reusable command\n");
		return -1;
	}

	if( bind(sock,(struct sockaddr *)&server , sizeof(server)) < 0) {
		printf("Failed to bind socket\n");
		return -1;
	}
	while(1) {
		to_recv_data(&sock, &data);
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
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);

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
#if 0
		result = recvfrom(sock , (char*)data, sizeof(packets), 0, (struct sockaddr *) &si_other, &slen );
		if (result <= 0 )
		{
			printf ( "Failed to receive command\n" );
			return result;
		}
#else
		data->data_length = 512;
#endif
		if (data->data_length > 0) {
			data->data = (char * ) malloc(data->data_length);
			if (data->data == NULL) {
				printf("Failed to allocate memory\n");
				return -1;
			}
			result = recvfrom(sock , data->data, data->data_length, 0, (struct sockaddr *) &si_other, &slen );
			if (result <= 0 )
			{
				printf ( "Failed to receive data\n" );
				free(data->data);
				return result;
			}
			printf("Recv string: %s from ip %s\n", data->data, inet_ntoa(si_other.sin_addr));
			free(data->data);
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

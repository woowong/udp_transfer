#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

void recv_file(int serv_sock);

int main (int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;


	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;


	if (argc!=2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);

	memset (&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

	clnt_addr_size = sizeof(clnt_addr);
	//	sendto(serv_sock, msg, sizeof(msg), 0, (struct sockaddr *)&clnt_addr, clnt_addr_size);
	//	write(serv_sock, msg, sizeof(msg));
	while(1) {
//		recvfrom (serv_sock, msg, sizeof(msg)-1, 0, (struct sockaddr *)&clnt_addr, clnt_addr_size); 
//		printf("received message is %s \n", msg);
		recv_file(serv_sock);
	}
	close(serv_sock);
	return 0;
}

void recv_file(int serv_sock)
{
	char recvBuffer[BUFFER_SIZE];
	char filename[BUFFER_SIZE];

	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	
	recvfrom(serv_sock, filename, sizeof(filename)-1, 0, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
	printf("Recevieving filename : %s\n", filename);

	int fd = open(filename, O_WRONLY | O_CREAT, 0755);
	int read_byte;
	do {
		recvfrom(serv_sock, recvBuffer, sizeof(recvBuffer)-1, 0, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
	read_byte = strlen(recvBuffer);
	write(fd, recvBuffer, read_byte);
	printf("Received Part : %s\n", recvBuffer);
	} while(read_byte > 0);
	close(fd);

}

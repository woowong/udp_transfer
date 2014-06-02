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
	
	recv_file(serv_sock);

	close(serv_sock);
	return 0;
}

void recv_file(int serv_sock)
{
	char recvBuffer[BUFFER_SIZE];
	char filename[BUFFER_SIZE];
	char ack[] = "ACK\0";
	int filesize;
	memset (filename, 0, sizeof(filename));
	memset (recvBuffer, 0, sizeof(recvBuffer));

	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	
	// filename get
	recvfrom(serv_sock, filename, sizeof(filename)-1, 0, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
	printf("Receiving filename : %s\n", filename);

	// filesize get
	recvfrom(serv_sock, recvBuffer, sizeof(recvBuffer)-1, 0, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
	sscanf(recvBuffer, "%d", &filesize);	
	printf("Receiving filesize : %d\n", filesize);

	int fd = open(filename, O_WRONLY | O_CREAT, 0755);
	int read_byte, total_byte=0;
	memset (recvBuffer, 0, sizeof(recvBuffer));
	while(1) {
		recvfrom(serv_sock, recvBuffer, sizeof(recvBuffer)-1, 0, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
		read_byte = strlen(recvBuffer);
		write(fd, recvBuffer, read_byte);
		sendto(serv_sock, ack, strlen(ack), 0, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
	//	printf("Received Part : %s\n", recvBuffer);
		total_byte += read_byte;
		memset (recvBuffer, 0, sizeof(recvBuffer));
		printf("totalbyte = %d \t filesize = %d\n", total_byte, filesize);
		if (total_byte == filesize)
			break;
	} 
	close(fd);
}


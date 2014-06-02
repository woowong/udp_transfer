#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

void send_file(int sock, struct sockaddr_in *serv_addr, char *filename);

int main(int argc, char* argv[])
{
	int sock;
	int str_len;
	struct sockaddr_in serv_addr;

	if (argc!=4) {
		printf("Usage : %s <IP> <port> <filename>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

//	char msg[] = "Hello World!";
//	sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	//send_file(sock, &serv_addr, argv[3]);
	char sendBuffer[BUFFER_SIZE];
	printf("argv[3] is %s\n", argv[3]);
	sendto(sock, argv[3], strlen(argv[3]), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	int fd = open(argv[3], O_RDONLY);
	int read_byte;
	while( (read_byte = read(fd, sendBuffer, sizeof(sendBuffer)) ) > 0) {
		sendto(sock, sendBuffer, read_byte, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		printf("Sended msg : %s\n", sendBuffer);
	}
	close(fd);

	close(sock);
	return 0;
}
/*
void send_file(int sock, struct sockaddr_in *serv_addr, char *filename)
{
	char sendBuffer[BUFFER_SIZE];
	socklen_t serv_addr_len = sizeof(*serv_addr);
	printf("filename is %s\n", filename);
	sendto(sock, filename, strlen(filename), 0, (struct sockaddr *)serv_addr, &serv_addr_len);

	int fd = open(filename, O_RDONLY);
	int read_byte;
	while( (read_byte = read(fd, sendBuffer, sizeof(sendBuffer)) ) > 0) {
		sendto(sock, sendBuffer, read_byte, 0, (struct sockaddr *)serv_addr, &serv_addr_len);
		printf("Sended msg : %s\n", sendBuffer);
	}
	close(fd);
}
*/

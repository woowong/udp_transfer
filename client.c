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

	// send filename
	char sendBuffer[BUFFER_SIZE];
	memset(sendBuffer, 0, sizeof(sendBuffer));
	printf("argv[3] is %s\n", argv[3]);
	sendto(sock, argv[3], strlen(argv[3]), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	int fd = open(argv[3], O_RDONLY); // open file
	
	// send file size
	int file_size = lseek(fd, 0, SEEK_END);
	printf("the file size : %d\n", file_size);
	sprintf(sendBuffer, "%d", file_size);
	sendto(sock, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	lseek(fd, 0, SEEK_SET); // set file offset start
	memset(sendBuffer, 0, sizeof(sendBuffer));
	int read_byte;
	while( (read_byte = read(fd, sendBuffer, sizeof(sendBuffer)-1)) > 0) {
		sendto(sock, sendBuffer, read_byte, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	//	printf("readbyte : %d\tSended msg : %s\n", read_byte, sendBuffer);
		memset(sendBuffer, 0, sizeof(sendBuffer));
	}
	close(fd);

	close(sock);
	return 0;
}

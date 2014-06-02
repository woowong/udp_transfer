#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUFFER_SIZE 1025
#define HEADER_SIZE 7 

unsigned short gen_checksum(char *buffer, int length);

int main(int argc, char* argv[])
{
	int sock;
	int str_len;
	int seq_num = 0;

	char recvBuffer[BUFFER_SIZE];
	char sendBuffer[BUFFER_SIZE + HEADER_SIZE];
	char dataBuffer[BUFFER_SIZE];

	struct sockaddr_in serv_addr;
	socklen_t serv_addr_len = sizeof(serv_addr);

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
	memset(sendBuffer, 0, sizeof(sendBuffer));
	printf("Sending File : %s\n", argv[3]);
	sendto(sock, argv[3], strlen(argv[3]), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	int fd = open(argv[3], O_RDONLY); // open file
	
	// send file size
	int file_size = lseek(fd, 0, SEEK_END);
	printf("File size : %d\n", file_size);
	sprintf(sendBuffer, "%d", file_size);
	sendto(sock, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	lseek(fd, 0, SEEK_SET); // set file offset start
	memset(sendBuffer, 0, sizeof(dataBuffer));
	memset(sendBuffer, 0, sizeof(sendBuffer));

	int read_byte;
	while( (read_byte = read(fd, dataBuffer, sizeof(dataBuffer)-1)) > 0) {
		// attach the header
		unsigned short checksum = gen_checksum(dataBuffer, read_byte);
		sprintf(sendBuffer, "%d %04hx %s", seq_num, checksum, dataBuffer);
		// send file data
		sendto(sock, sendBuffer, read_byte + HEADER_SIZE, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
//		printf("read_byte = %d\n", read_byte);
		// receive ACK
		recvfrom(sock, recvBuffer, sizeof(recvBuffer)-1,0, (struct sockaddr *)&serv_addr, &serv_addr_len);
		printf("checksum : %hx\t readbyte : %d\n", checksum, read_byte);
		memset(dataBuffer, 0, sizeof(dataBuffer));
		memset(sendBuffer, 0, sizeof(sendBuffer));
	}
	close(fd);
	printf("\nProgram End.\n");
	close(sock);
	return 0;
}

unsigned short gen_checksum(char *buffer, int length)
{
	unsigned int checksum = 0;
	int i=0;
	unsigned int c;
	for ( ; length > 0; length -= 2)
	{
		c += (unsigned int)buffer[i] << 8;
		if (length != 1) 
			c += (unsigned int)buffer[i+1];
		c = (c&0xFFFF) +  (c>>16); // carry bit
		i += 2;
	}
	return (unsigned short) ~c;
}


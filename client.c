#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/time.h>

#define BUFFER_SIZE 1025
typedef struct datagram {
	int seq_num;
	uint16_t checksum;
	char dataBuffer[BUFFER_SIZE];
	int read_byte;
} DATA;

uint16_t gen_checksum(char *buffer, int length);

int main(int argc, char* argv[])
{
	int sock;
	int str_len;
	struct sockaddr_in serv_addr;
	int ack = 1;
	int temp;
	DATA packet;
	char sendBuffer[BUFFER_SIZE];
	char recvBuffer[BUFFER_SIZE];
	socklen_t serv_addr_len = sizeof(serv_addr);

	if (argc!=4) {
		printf("Usage : %s <IP> <port> <filename>\n", argv[0]);
		exit(1);
	}
	
	memset(&packet, 0, sizeof(packet));
	//packet.seq_num = 0;

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	// send filename
	memset(sendBuffer, 0, sizeof(sendBuffer));
	printf("Sending File : %s\n", argv[3]);
	sendto(sock, argv[3], strlen(argv[3]), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	recvfrom(sock, &temp, sizeof(temp), 0, (struct sockaddr *)&serv_addr, &serv_addr_len);
	
	int fd = open(argv[3], O_RDONLY); // open file
	
	// send file size
	int file_size = lseek(fd, 0, SEEK_END);
	printf("File size : %d\n", file_size);
	sprintf(sendBuffer, "%d", file_size);
	sendto(sock, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	recvfrom(sock, &temp, sizeof(temp), 0, (struct sockaddr *)&serv_addr, &serv_addr_len);

	lseek(fd, 0, SEEK_SET); // set file offset start
	memset(sendBuffer, 0, sizeof(sendBuffer));
	int read_byte;
	int ack_err = 0;
	printf("File is being sended...\n");
	while(1) {
		if(!ack_err) {	
			read_byte = read(fd, packet.dataBuffer, sizeof(packet.dataBuffer)-1);
			if(read_byte <= 0) break;
			packet.seq_num = !ack;
			packet.read_byte = read_byte;
			packet.checksum = gen_checksum(packet.dataBuffer, read_byte);
		}
		// send file data
		sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		// receive ACK
		if ( recvfrom(sock, &ack, sizeof(ack), 0, (struct sockaddr *)&serv_addr, &serv_addr_len) < 0 ) {
			printf("ERROR : Time out.\n");
			ack_err = 1;
		}
		ack_err = (ack!=packet.seq_num)	? 1 : 0;
		if(!ack_err) 
			memset(packet.dataBuffer, 0, sizeof(packet.dataBuffer));
	}
	close(fd);
	printf("File Sending is over.\n");
	close(sock);
	return 0;
}

uint16_t gen_checksum(char *buffer, int length)
{
	uint16_t *buf = (void *)buffer;
	uint32_t sum = 0;
	while ( length > 1 )
	{
		sum += *buf++;
		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) +(sum >> 16);
		length -= 2;
	}
	if ( length == 1 )
		sum += *((uint8_t *)buf);
	while ( sum >> 16 )
		sum = (sum & 0xFFFF) +(sum >> 16);
	return (uint16_t)(~sum);
}


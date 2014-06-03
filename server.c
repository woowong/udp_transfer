#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUFFER_SIZE 1025
typedef struct datagram {
	int seq_num;
	uint16_t checksum;
	char dataBuffer[BUFFER_SIZE];
	int read_byte;
} DATA;

void recv_file(int serv_sock);
uint16_t check_checksum(char *buffer, int length, uint16_t checksum);

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
	DATA packet;
	char recvBuffer[BUFFER_SIZE];
	char filename[BUFFER_SIZE];
//	char ack[] = "ACK\0";
	int ack;

	int filesize;
	memset (filename, 0, sizeof(filename));
	memset (recvBuffer, 0, sizeof(recvBuffer));

	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	
	//initial packet
	memset(&packet, 0, sizeof(packet));
	
	// filename get
	recvfrom(serv_sock, filename, sizeof(filename)-1, 0, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
	sendto(serv_sock, &ack, sizeof(ack), 0, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
	printf("Receiving filename : %s\n", filename);

	// filesize get
	recvfrom(serv_sock, recvBuffer, sizeof(recvBuffer)-1, 0, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
	sendto(serv_sock, &ack, sizeof(ack), 0, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
	sscanf(recvBuffer, "%d", &filesize);	
	printf("Receiving filesize : %d\n", filesize);

	int fd = open(filename, O_WRONLY | O_CREAT, 0755);
	int read_byte, total_byte=0;
	int chk_err = 0, dup_err = 0, timeout_err = 0;
	int prev_seq_num = 1;
	memset (recvBuffer, 0, sizeof(recvBuffer));
	printf("File is being received...\n");
	while(1) {
		// get packet
		recvfrom(serv_sock, &packet, sizeof(packet), 0, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
		read_byte = packet.read_byte;

		chk_err = (check_checksum(packet.dataBuffer, read_byte, packet.checksum) != 0) ? 1 : 0; // checksum err
		dup_err = packet.seq_num == prev_seq_num ? 1 : 0; // duplicate
	// timeout_err

		if(!chk_err & !dup_err & !timeout_err) { // get success
			ack = packet.seq_num;
			write(fd, packet.dataBuffer, read_byte); // wrtie to file
			total_byte += read_byte;
			sendto(serv_sock, &ack, sizeof(ack), 0, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
			prev_seq_num = packet.seq_num;
			if (total_byte == filesize)
				break;
		}
		else if(chk_err) { // checksum error occured
			ack = !ack;
			sendto(serv_sock, &ack, sizeof(ack), 0, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
			printf("ERROR : Checksum is corrupted.\n");
		}
		else if(dup_err) { // duplicate error occured
			ack = !ack;
			sendto(serv_sock, &ack, sizeof(ack), 0, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
			printf("ERROR : Duplicate sequence number.\n");
		}
/*		else if(timeout_err) { // timeout error occured
			sendto(serv_sock, &ack, sizeof(ack), 0, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
			printf("ERROR : Time out receiving.\n");
		}*/
		memset(&packet, 0, sizeof(packet));
		memset(recvBuffer, 0, sizeof(recvBuffer));
	}
	close(fd);
	printf("File Receiving is over.\n");
	close(serv_sock);
}

uint16_t check_checksum(char *buffer, int length, uint16_t checksum)
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
	return (uint16_t)sum + checksum + 1;
}



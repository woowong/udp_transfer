#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling (char *msg);

int main (int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;


	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	char msg[30];
//	char msg[] = "Hello World!";

	if (argc!=2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (serv_sock == -1)
		error_handling ("socket() error");

	memset (&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
		error_handling ("bind() error");
		
	clnt_addr_size = sizeof(clnt_addr);
//	sendto(serv_sock, msg, sizeof(msg), 0, (struct sockaddr *)&clnt_addr, clnt_addr_size);
//	write(serv_sock, msg, sizeof(msg));
while(1) {
	recvfrom (serv_sock, msg, sizeof(msg)-1, 0, (struct sockaddr *)&clnt_addr, clnt_addr_size); 
	printf("received message is %s \n", msg);
}
	close(serv_sock);
	return 0;
}

void error_handling (char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
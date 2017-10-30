#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main()
{
	int client_socket;
	struct sockaddr_in server_addr;
	
	char recvbuf[200];
	char sendbuf[200];
	
	//创建套接字
	if((client_socket = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		printf("创建失败！\n");
		return 0;
	}

	//发起连接
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5050);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	if(connect(client_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
	{
		printf("连接失败！\n");
		return 0;
	}

	printf("connect with host...\n");

	//数据收发
	while(1)
	{
		printf("Input your word: \n");
		scanf("%s",sendbuf);
		
		send(client_socket,sendbuf,sizeof(sendbuf),0);

		recv(client_socket,recvbuf,200,0);
		printf("recv data: %s",recvbuf);
		printf("\n");
	}
	
	close(client_socket);
	return 0;
}

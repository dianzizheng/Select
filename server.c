#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define QUEUE_LENGTH 5

int main()
{
	int server_socket;
	int new_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	fd_set detectionset;
	int client_socket[QUEUE_LENGTH];
	struct timeval tv;
	socklen_t sin_size = sizeof(client_addr);
	char buf[200];
	int ret;
	int i;
	int conn_num = 0;

	//创建服务套接字
	if((server_socket = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		printf("创建失败！\n");
		return 0;
	}

	//设置套接字
	int yes = 1;
	if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
	{
		printf("设置失败！\n");
		return 0;
	}

	//服务绑定
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5050);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	memset(server_addr.sin_zero,'\0',sizeof(server_addr.sin_zero));
	
	if(bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
	{
		printf("绑定失败！\n");
		return 0;
	}

	//服务侦听
	if(listen(server_socket,5) < 0)
	{
		printf("侦听失败！\n");
		return 0;
	}

	printf("listen port %d\n",5050);

	int maxsock = server_socket;
	while(1)
	{	
		//将服务套接字添加到检测集s合
		FD_ZERO(&detectionset);
		FD_SET(server_socket,&detectionset);
	
		//设置时间
		tv.tv_sec = 15;
		tv.tv_usec = 0;

		//将客户端套接字集合中的可用套接字添加到检测集合
		for(i = 0; i < QUEUE_LENGTH; i++)
		{
			if(client_socket[i] != 0)
			{			
				FD_SET(i,&detectionset);
			}
		}
			
		//设置select
		ret = select(maxsock+1,&detectionset,NULL,NULL,&tv);
		if(ret < 0)
		{
			printf("select失败！\n");
			break;
		}
		else if(ret == 0)
		{
			printf("等待时间！\n");
			continue;
		}
		
		//判断当前是否有可读得客户端套接字
		for(i=0;i<conn_num;i++)
		{
			if(FD_ISSET(client_socket[i],&detectionset))
			{
				//读取数据
				ret = recv(client_socket[i],buf,sizeof(buf),0);
				if(ret <= 0)
				{
					printf("接收失败！\n");
					close(client_socket[i]);		
					FD_CLR(client_socket[i],&detectionset);
					client_socket[i]=0;
				}
				else
				{
					printf("接收数据：%s\n",buf);
					//发送数据
					send(client_socket[i],buf,sizeof(buf),0);
				}
			}
		}
		
		//判断当前是否有新连接
		if(FD_ISSET(server_socket,&detectionset))
		{
			new_socket = accept(server_socket,(struct sockaddr*)&client_addr,&sin_size);
			if(new_socket <= 0)
			{
				printf("建立链接失败！\n");
				continue;
			}
			
			//若有新连接，加入到客户端套接字集合
			if(conn_num < QUEUE_LENGTH)
			{
				client_socket[conn_num++] = new_socket;			
				printf("new client[%d] %s:%d\n",conn_num,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
				if(new_socket > maxsock)
					maxsock = new_socket;
			}
			//若数量过载，则断开本次连接，并发送提示信息“sorry overload!”
			else
			{
				send(new_socket,"sorry overload!",sizeof("sorry overload!"),0);
	
				//关闭本次连接
				close(new_socket);
				break;
			}			
		}
		
	}

	for(i = 0;i < QUEUE_LENGTH;i++)
	{
		if(client_socket[i] != 0)
			close(client_socket[i]);
	}
}

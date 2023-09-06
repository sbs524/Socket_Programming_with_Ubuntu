#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>

#define MAX_CLIENT 2
#define BUFF_SIZE 300

int CLIENT_INDEX = 0;
char buff [BUFF_SIZE];
int client_list[MAX_CLIENT];

pthread_mutex_t g_mutex;


void *create_client(void *socket)
{
	int sock = (int)socket;
	char msg[BUFF_SIZE];
	char id[30];
	int isTrue = 1;
	int i=0;

	if( read(sock, id, 30) == -1)
	{
		perror("id를 받아올 수 없음\n");
		isTrue = 0;
	}	
	else
		printf("%s님이 입장하였습니다.\n", id);

	while(isTrue)
	{
		if( read(sock, msg, BUFF_SIZE) == -1)
		{
			perror("글을 읽어올 수 없음\n");
			break;
		}

		if( strcmp(msg, "-1") == 0)  break;

		printf("%s\n", msg);

		pthread_mutex_lock(&g_mutex);
		for( i=0;i<CLIENT_INDEX;i++)
		{
			if( client_list[i] == sock)  continue;
			write(client_list[i], msg, strlen(msg)+1);
		}
		pthread_mutex_unlock(&g_mutex);
	}

	pthread_mutex_lock(&g_mutex);
	sprintf(msg, "%s님이 퇴장하였습니다.\n", id);
	for(i=0;i<CLIENT_INDEX - 1;i++)
	{
		if( client_list[i] == sock)
		{
			for(;i<CLIENT_INDEX - 1;i++)
				client_list[i] = client_list[i+1];
			write(client_list[i], msg, strlen(msg)+1);
			break;
		}
		write(client_list[i], msg, strlen(msg)+1);
	}

	printf("%s",  msg);
	CLIENT_INDEX--;
	pthread_mutex_unlock(&g_mutex);
	close(sock);
	printf("총 %d명이 참가할 수 있고 현재 %d명이 참가중입니다.\n", MAX_CLIENT, CLIENT_INDEX);
	pthread_exit(0);
	return NULL;
}


int main(int argc, char* argv[]){

	if(argc<=1)
	{
		printf("포트번호를 입력해주세요.\n");
		exit(1);
	}


	int serv_sock;
	int client_sock;
	int serv_len, client_len;
	struct sockaddr_in serv_address;
	struct sockaddr_in client_address;
	pthread_t sock_thread;

	pthread_mutex_init(&g_mutex, NULL);

	serv_sock = socket(AF_INET, SOCK_STREAM, 0);

	if(serv_sock == -1)
	{
		perror("socket problem");
		exit(1);
	}

	memset(&serv_address, 0x00, sizeof(serv_address));
	serv_address.sin_family = AF_INET;
	serv_address.sin_port = htons(atoi(argv[1]));
	serv_address.sin_addr.s_addr = inet_addr(argv[2]);
	serv_len = sizeof(serv_address);

	if( bind(serv_sock, (const struct sockaddr*) &serv_address, serv_len)  == -1)
	{
		perror("bind problem");
		exit(1);
	}

	printf("%s\n", inet_ntoa(serv_address.sin_addr));
	if( listen(serv_sock, 5) == -1)
	{
		perror("listen problem");
		exit(1);
	}



	while(1)
	{
		printf("총 %d명이 참가할 수 있고 현재 %d명이 참가중입니다.\n", MAX_CLIENT, CLIENT_INDEX);
		client_len = sizeof(client_address);
		client_sock = accept(serv_sock, (struct sockaddr*)&client_address, &client_len);

		if(client_sock == -1)
		{
			perror("소켓과 서버와의 연결이 실패하였습니다.\n");
			continue;
		}

		if( CLIENT_INDEX == MAX_CLIENT)
		{
			printf("더 이상 인원을 수용할 수 없습니다.\n");
			write(client_sock, "-1", 3);
			continue;
		}
		else
			write(client_sock, "welcome", 8);

		pthread_mutex_lock(&g_mutex);
		client_list[CLIENT_INDEX++] = client_sock;
		pthread_mutex_unlock(&g_mutex);

		pthread_create(&sock_thread, NULL, create_client, (void*)client_sock);
	}


	return 0;
}





#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>


#define MSG_SIZE 330
#define BUFF_SIZE 300
char buff[BUFF_SIZE];
char msg[MSG_SIZE];

void *rcv_msg_system(int socket)
{
	int sock = (int)socket;
	char rcv_msg[MSG_SIZE];

	while(1)
	{
		if( read(sock, rcv_msg, MSG_SIZE) == -1)
		{
			perror("다른 사람이 보낸 채팅을 받을 수 없습니다.\n");
			break;
		}
		printf("%s\n", rcv_msg);
	}

	pthread_exit(0);
	return NULL;
}


int main()
{
	int client_sock;
	int len;
	int port;	//포트값
	char ip[20];	//ip주소 값
	char id[30];	//사용자 id값
	int isTrue;	//입력값의 정상유무
	struct sockaddr_in client_address;
	pthread_t sock_thread;

	while(1){

		printf("포트번호 : ");
		if(scanf("%d", &port) == 1)
			break;
		else
		{
			while(isTrue = getchar() != '\n' && isTrue != EOF);
			printf("숫자를 입력해주세요\n\n");
		}
	}

	printf("ip주소 : ");
	scanf("%s", ip);

	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	if(client_sock == -1)
	{
		perror("socket prolem\n");
		exit(1);
	}

	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(port);
	client_address.sin_addr.s_addr = inet_addr(ip);
	len = sizeof(client_address);

	if(connect(client_sock, (const struct sockaddr*) &client_address, len) == -1)
	{
		perror("connect problem\n");
		exit(1);
	}

	read(client_sock, buff, BUFF_SIZE);
	if( strcmp(buff, "-1") == 0)
	{
		printf("서버에서 더이상 고객을 수용할 수 없습니다.");
		exit(1);
	}


	printf("id : ");
	scanf("%s", id);			
	write(client_sock, id, strlen(id)+1);
	printf("%s님이 입장하였습니다. 이제부터 자유롭게 대화를 나누시면 됩니다.\n대화종료를 원하면 \"-1\"을 입력하세요.\n\n" , id);	
	while(isTrue = getchar() != '\n' && isTrue != EOF);
	pthread_create(&sock_thread, NULL, rcv_msg_system, (void *)client_sock);

	while(1)
	{
		gets(buff);
		if (strcmp(buff, "-1") == 0)
		{
			write(client_sock, buff, 3);
			break;
		}

		strcpy(msg , "[ ");
		strcat(msg, id);
		strcat(msg, " ] : ");
		strcat(msg, buff);

		write(client_sock, msg, strlen(msg)+1);

	}

	close(client_sock);
	return 0;

}


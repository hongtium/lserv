#ifndef __DDAP_H_
#define __DDAP_H_

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#define UDPPORT 13000

//#define UDP_CLIENT_PORT 14000
#define SERVER_QUEUE 65 
#define UDPSERVERS 39
#define FUNCTIMEOUT 2000
#define GETUSERITEMTIMEOUT 5

//ms

#define UDP_SERVERS 30 
//#define UDP_SERVER_PORT 13000
#define UDP_SERVER_TIMEOUT 2 

#define UDPTIMEOUT 100

#define SUCCESS 0
#define FAIL -23231

#define SERVERIPS	"192.168.1.65",""
#define SERVERID	0	//当前程序所用的server地址
#define SERVERS		1	//servers

#define sem_init_1(a,b) sem_init((a),0,1);

#define DEBUG_OUTPUT	1


typedef struct
{
	int start;
	int end;
	int ListenSocket;
	int SendSocket;
	int bind_socket;
	struct sockaddr_in in_Addr;
	struct sockaddr_in out_Addr;
	char OutBuffer[65536];
	int OutLen;
	char InBuffer[65536];
	int InLen;
	pthread_t Thread;
	char InUse;
	//sem_t MyLock;
	sem_t *toOpen;
	int timeout;
	int id;
	unsigned short sn;
} Server;


void * udpServerBody(void * n);

int recivePackage(Server * my);

int sendMe(Server * my);

int runFunction(Server * my);

int getIDHost(char *userid);
int getIDHostIP(char *userid);
int doDummy(Server *my);
int doDummyRet(Server *my,int ret);
short getFunction(Server * my);
int getPkgLen(Server * my);
short getPkgSn(Server * my);
short getPkgPort(Server * my);
int lockServer(short port);
int initServer(short port);

char *strtime(char *str,int strlen);
#endif

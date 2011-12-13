
#include "ddap.h"
#include "service.h"
#include "misc.h"

#include <sys/stat.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h> //flock



/*protocol
inPkg:
+0	+2	+4		+6	+10
<port>	<sn>	<function>	<len>	[data]
2B	2B	2B		4B	lenB
outPkg:
+0	+2		+6
<sn>	<len/ret>	[data]
2B	4B		lenB
*/




int Timeout = 0;
int RecodeRequest = 0;
Server * svs;
Server ** MyQueue;
sem_t * initLock;
int t_Index;
int ttw = 0;
sem_t * QueueLock;
sem_t * QueueEmpty;
sem_t * QueueOutLock;
sem_t * timeoutlock;
sem_t * ef_lock;
static unsigned long rrq,wrq;
static unsigned long dcount = 0; 
static unsigned long Read = 0,Write = 0; 
#define SP_MOD 60
static unsigned long reqrl[SP_MOD]; 
static unsigned long reqwl[SP_MOD]; 

int pkgs,pkg_errs;
int totalFiles = 0;
int badPackage;
int SystemStatus = 0;
extern char *program;
int *servers;
int lockServer(short port)
{
	int p = socket(AF_INET,SOCK_DGRAM,0);
	struct sockaddr_in o;
	bzero(&o,sizeof(o));
	o.sin_family = AF_INET;
	o.sin_port = port;
	if(bind(p,(struct sockaddr *)&o,sizeof(o)))
	{
		debug_output(stderr,"%s:Other program runing\n",program);	
		return 1;
	}
	return 0;
}

void * Monitor(void * v)
{
	int reset = 0;
	unsigned int checkpoint_n = 0;
	while(1)
	{
		char * s;
		int vv;
		
		FILE * fp = fopen("lserv.log","a");
		sem_getvalue(QueueEmpty,&vv);
		dcount++;
        	fprintf(fp,"Qe=%d Tmo %d PKG %d\n",vv,Timeout,pkgs);
		reqrl[dcount%SP_MOD] = Read;
		reqwl[dcount%SP_MOD] = Write;
		
		fclose(fp);
		sleep(1);
		if(vv < 10)reset++;
		else
			reset = 0;
		if(reset>10)
		{
                        debug_output(stderr,"Fatal err,Queues are exhausted\n");
                        debug_output(stderr,"Fatal err,Queues are exhausted\n");
			exit(0);
		}
	}
}

int initServer(short port)
{
	int x1 = 0,num=0;
	int s = socket(AF_INET,SOCK_DGRAM,0);
	struct sockaddr_in a;
	char *IPs[] = {SERVERIPS};
	int init_s;
	Server * my;
	pthread_t MonitorThread;
	while(IPs[x1++][0] != 0);
	servers = (int *)malloc(sizeof(int) * x1);
	for(x1 = 0 ; IPs[x1][0] != 0;x1++)
	{
		struct in_addr tmp;
		inet_aton(IPs[x1],&tmp);
		servers[x1] = ntohl(tmp.s_addr);
		debug_output(stdout,"%s:server[%d]:%s\n",program,x1,IPs[x1]);
	}
	bzero(&a,sizeof(struct sockaddr_in));
	a.sin_family = AF_INET;
	a.sin_port=htons(port);
	if(bind(s,(struct sockaddr *)&(a),sizeof(struct sockaddr_in)))
        {
               debug_output(stdout,"%s:INIT UDP FATAL ERROR!\n",program);
               exit(1);
        }
	debug_output(stdout,"%s:BIND UDP:%d OK\n",program,port);

	svs = (Server *)calloc(sizeof(Server)*UDPSERVERS,1);
	initLock = (sem_t *)malloc(sizeof(sem_t));
	QueueLock = (sem_t *)malloc(sizeof(sem_t));
	QueueOutLock = (sem_t *)malloc(sizeof(sem_t));
	QueueEmpty = (sem_t *)malloc(sizeof(sem_t));
	timeoutlock = (sem_t *)malloc(sizeof(sem_t));
	ef_lock = (sem_t *)malloc(sizeof(sem_t));

	MyQueue = (Server **)malloc(sizeof(Server *)*SERVER_QUEUE);

	sem_init(QueueLock,0,0);
	sem_init(QueueEmpty,0,SERVER_QUEUE);
	sem_init(QueueOutLock,0,1);
	sem_init(timeoutlock,0,1);
	sem_init(ef_lock,0,1);

	for(x1 = 0 ; x1 < SERVER_QUEUE ; x1 ++)
	{
		MyQueue[x1] = (Server *)malloc(sizeof(Server));
		MyQueue[x1] -> ListenSocket = s;
		MyQueue[x1] -> id = x1;
		MyQueue[x1] -> SendSocket = socket(AF_INET,SOCK_DGRAM,0);	
		bzero(MyQueue[x1] -> InBuffer,sizeof(MyQueue[x1]->InBuffer));
		bzero(MyQueue[x1] -> OutBuffer,sizeof(MyQueue[x1]->OutBuffer));
		MyQueue[x1] -> InLen = 0;
		MyQueue[x1] -> OutLen = 0;
		MyQueue[x1] -> InUse = 0;
	}
	bzero(reqrl,SP_MOD);
	bzero(reqwl,SP_MOD);

	sem_init(initLock , 0 ,0);

	pthread_create(&MonitorThread,NULL,Monitor,&x1);
	debug_output(stdout,"%s:Create Monitor Thread OK\n",program);

	for(x1 = 0 ; x1 < UDPSERVERS ; x1 ++)
	{
		svs[x1].id = x1;
		pthread_create(&(svs[x1].Thread),NULL,udpServerBody,&x1);
#ifndef GDB
		sem_wait(initLock);
#else
		{int rv;do { rv = sem_wait(initLock); } while ((rv == -1) && (errno == EINTR));}
#endif
		debug_output(stdout,"%s:Create Thread[%d] OK\n",program,x1);
	}	

        debug_output(stdout,"%s@%d:INIT OK\n",program,port);
	
	while(1)
	{
		struct timeb tp;
#ifndef GDB
		sem_wait(QueueEmpty);
#else
		{int rv;do { rv = sem_wait(QueueEmpty); } while ((rv == -1) && (errno == EINTR));}
#endif
#ifndef GDB
		sem_wait(QueueOutLock);
#else
		{int rv;do { rv = sem_wait(QueueOutLock); } while ((rv == -1) && (errno == EINTR));}
#endif


		
		while((MyQueue[num%SERVER_QUEUE])->InUse)num ++;
		MyQueue[num%SERVER_QUEUE]->InUse = 1;

		sem_post(QueueOutLock);

		my = MyQueue[num%SERVER_QUEUE];
		recivePackage(my);

		ftime(&tp);
		my->start = (tp.time%1000000)*1000 +tp.millitm; 	
		sem_post(QueueLock);
	}
}

void * udpServerBody(void * n)
{
	Server * my , * mmm = &svs[t_Index];
	int num = 0;
	int timeoutlong = 2000;
	int ret = -1;

	sem_post(initLock);

	while(1)
	{
		struct timeb tp;
		int Qid = 0;
		int isBadPkg = 0;
#ifndef GDB
		sem_wait(QueueLock);
#else
		{int rv;do { rv = sem_wait(QueueLock); } while ((rv == -1) && (errno == EINTR));}
#endif
#ifndef GDB
		sem_wait(QueueOutLock);
#else
		{int rv;do { rv = sem_wait(QueueOutLock); } while ((rv == -1) && (errno == EINTR));}
#endif

		while(MyQueue[num%SERVER_QUEUE]->InUse != 2)num ++;
		MyQueue[num%SERVER_QUEUE]->InUse = 3;
		Qid = num%SERVER_QUEUE;
		my = MyQueue[Qid];
		sem_post(QueueOutLock);

		if((my->InLen) <= 0)continue;
		ret = runFunction(my);

		ftime(&tp);
		my->end = (tp.time%1000000)*1000 +tp.millitm; 

		if(my->end-my->start>timeoutlong)
		{

#ifndef GDB
			sem_wait(timeoutlock);
#else
			{int rv;do { rv = sem_wait(timeoutlock); } while ((rv == -1) && (errno == EINTR));}
#endif
			{
				FILE * fp = fopen("ddap.timeout.log","a");
				char time[128];
				fprintf(fp,"%s,TIMEOUT: func = %d, time = %d\n",strtime(time,sizeof(time)),getFunction(my),my->end-my->start);
				fclose (fp);
			}
			sem_post(timeoutlock);

			Timeout ++;
		}
		sem_post(QueueEmpty);
		MyQueue[Qid]->InUse = 0;
	}
}

int runFunction(Server * my)
{
	int Function = getFunction(my);
	int ret=-1,vv;

	sem_getvalue(QueueEmpty,&vv);
	my->timeout = FUNCTIMEOUT;

	if(0)
	{
		ret = doDummy(my);
	}
	else
	{
		//printf("runFunction:%d\n",Function);
		switch(Function)
		{
			case 0x0001:
				ret = do_log(my);
			break;
			case 0x0011:
				ret = do_log_str(my);
			break;
			case 0x0012:
				ret = do_log_int(my);
			break;

		}

	}	
	/*
	if(ret > 0)sendMe(my);	
	else
	{
		doDummyRet(my,ret);
		sendMe(my);
	}
	*/
}

int doDummy(Server *my)
{
	return doDummyRet(my,0);
}
int doDummyRet(Server *my,int ret)
{

	memcpy(my->OutBuffer,&ret,4);
	my->OutLen = 4;
	return 6;
}

int recivePackage(Server *my)
{
	socklen_t x1=sizeof(my->out_Addr);
	while(0>=(my->InLen = recvfrom(my->ListenSocket,my->InBuffer,65536,0,(struct sockaddr *)&(my->out_Addr),&x1)));
	my->InUse = 2;
	pkgs++;
#ifdef  DEBUGUDP
	printf ("Recive MQ[%d]:%d package from %s:%d(%#.4x:%dB)\n",my->id,getPkgSn,inet_ntoa(my->out_Addr.sin_addr),ntohs(my->out_Addr.sin_port),getFunction(my),my->InLen);
#endif
}

int sendMe(Server * my)
{
	int x1 = 2;
	short sn,port;
	char OutBuffer[65535];
	if(my->OutLen <=0)return -1;
	port = getPkgPort(my);
	sn = getPkgSn(my);	
	if(port)my->out_Addr.sin_port=htons(port);
	memcpy(OutBuffer + 0,&sn,2);
	memcpy(OutBuffer + 2,&my->OutLen,4);
	memcpy(OutBuffer + 6,my->OutBuffer,my->OutLen);

	sendto(my->SendSocket,OutBuffer,my->OutLen + 6,0,(struct sockaddr *)&(my->out_Addr),sizeof(my->out_Addr));

#ifdef  DEBUGUDP
	printf("Send MQ[%d]:%d package to %s:%d(%dB)\n",my->id,sn,inet_ntoa(my->out_Addr.sin_addr),ntohs(my->out_Addr.sin_port),my->OutLen);
#endif
}

short getFunction(Server * my)
{
	return *(short *)(my->InBuffer+2);
}
int getPkgLen(Server * my)
{
	return *(int *)(my->InBuffer+6);
}
short getPkgSn(Server * my)
{
	return *(short *)(my->InBuffer+4);
}
short getPkgPort(Server * my)
{
	return 0;
	//return *(short *)(my->InBuffer+0);
}
int getIDHost(char *userid)
{

	int hash = userid[0]*31 + userid[2]; 
	if(hash < 0) hash = (-1 * hash);
	return hash;
}
int getIDHostID(char *userid)
{
	return getIDHost(userid)%SERVERS;
}
int getIDHostIP(char *userid)
{

	return servers[getIDHostID(userid)];
}


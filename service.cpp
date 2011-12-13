#include "ddap.h"
#include "misc.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
       #include <sys/socket.h>
       #include <netinet/in.h>
       #include <arpa/inet.h>


int init_service()
{
	return 0;
}
int do_log(Server *my)
{

	char *log_mod= my->InBuffer + 10;
	unsigned int oper;
	memcpy(&oper,my->InBuffer+42,sizeof(oper));
	unsigned short len;
	memcpy(&len,my->InBuffer+46,sizeof(len));
	char *log_str= my->InBuffer + 48;
	
	char datestr[256];
	time_t tms = time(NULL);
        struct tm tt;
        struct tm *ttp = localtime_r(&tms,&tt);
        strftime(datestr,sizeof(datestr),"%Y%W",ttp);

	char logroot[]="/opt/logdata/";
	char filename[1024];
	filename[0] = 0;
	strcat(filename,logroot);
	strcat(filename,log_mod);
	mkdir(filename,0777);
	strcat(filename,"/");
	strcat(filename,datestr);
	strcat(filename,".log");

	FILE *fp = fopen(filename,"a");
	if(fp)
	{
		debug_output(fp,"%s\n",log_str);
	}
	else return -1;

	fclose(fp);
	
	return 1;
}
int do_log_str(Server *my)
{
	long long intv1;
	long long intv2;
	long long intv3;
	int IP;
	long long timestamp;
	char *userid,*suv,*ext;
	int off = 10;
	memcpy(&intv1,my->InBuffer + off,sizeof(intv1));
	off += sizeof(intv1);
	memcpy(&intv2,my->InBuffer + off,sizeof(intv2));
	off += sizeof(intv2);
	memcpy(&intv3,my->InBuffer + off,sizeof(intv3));
	off += sizeof(intv3);
	memcpy(&IP,my->InBuffer + off,sizeof(IP));
	off += sizeof(IP);
	memcpy(&timestamp,my->InBuffer + off,sizeof(timestamp));
	off += sizeof(timestamp);
	suv = my->InBuffer + off;
	off += 32;
	userid = my->InBuffer + off;
	if(!isuserid(userid))
		return -3;
	off += strlen(userid) + 1;
	ext = my->InBuffer + off;
	char loweruid[64];
	strncpy(loweruid,userid,sizeof(loweruid));
	StrToLower(loweruid, 0);
	cFile file(loweruid,".str");
	if(file.open("a"))
	{
		char timestr[128];
		struct sockaddr_in addr; 
		addr.sin_addr.s_addr = IP;
		debug_output(file.handle(),"%s,%qd,%qd,%qd,%s,%s,%s,%s\n",loweruid,intv1,intv2,intv3,inet_ntoa(addr.sin_addr),suv,long2time(timestamp/1000,timestr,sizeof(timestr)),ext);
	//	debug_output(stderr,"%s,%qd,%qd,%qd,%s,%s,%s,%s\n",userid,intv1,intv2,intv3,inet_ntoa(addr.sin_addr),suv,long2time(timestamp/1000,timestr,sizeof(timestr)),ext);
	}
	else
	{
		debug_output(stderr,"do_log_str:cFile.open(%s) err\n",file.filename().c_str());
		return -2;
	}
	return 1;

}
int do_log_int(Server *my)
{
	long long intv1;
	long long intv2;
	long long intv3;
	int IP;
	long long timestamp;
	char *userid,*suv,*ext;
	int off = 10;
	memcpy(&intv1,my->InBuffer + off,sizeof(intv1));
	off += sizeof(intv1);
	memcpy(&intv2,my->InBuffer + off,sizeof(intv2));
	off += sizeof(intv2);
	memcpy(&intv3,my->InBuffer + off,sizeof(intv3));
	off += sizeof(intv3);
	memcpy(&IP,my->InBuffer + off,sizeof(IP));
	off += sizeof(IP);
	memcpy(&timestamp,my->InBuffer + off,sizeof(timestamp));
	off += sizeof(timestamp);
	suv = my->InBuffer + off;
	off += 32;
	userid = my->InBuffer + off;
	off += strlen(userid) + 1;
	ext = my->InBuffer + off;
	char s_intv1[128];
	sprintf(s_intv1,"%qd",intv1);
	cFile file(s_intv1,".int");
	if(file.open("a"))
	{
		char timestr[128];
		struct sockaddr_in addr; 
		addr.sin_addr.s_addr = IP;
		debug_output(file.handle(),"%qd,%s,%qd,%qd,%s,%s,%s,%s\n",intv1,userid,intv2,intv3,inet_ntoa(addr.sin_addr),suv,long2time(timestamp/1000,timestr,sizeof(timestr)),ext);
	//	debug_output(stderr,"%qd,%s,%qd,%qd,%s,%s,%s,%s\n",intv1,userid,intv2,intv3,inet_ntoa(addr.sin_addr),suv,long2time(timestamp/1000,timestr,sizeof(timestr)),ext);
	}
	else
	{
		debug_output(stderr,"do_log_int:cFile.open(%s) err\n",file.filename().c_str());
		return -2;
	}
	return 1;
}

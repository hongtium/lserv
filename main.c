#include"ddap.h"
#include"service.h"
char *program;
int main(int args , char ** argv)
{
	program=argv[0];
	if(args==2)
	{
		short lisentport=atoi(argv[1]);
		if(lisentport)
		{
			if(lockServer(lisentport+1))exit(1);
			if(init_service())exit(1);
			initServer(lisentport);
		}
		else
			
			fprintf(stderr,"%s:lisentport:%d \n",program,lisentport);
	}
	else
		fprintf(stderr,"usage:%s lisentport \n",program);
		fprintf(stderr,"%s,args:%d\n",program,args);
}

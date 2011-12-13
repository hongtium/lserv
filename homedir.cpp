#include "misc.h"
#include<iostream>
int main(int args , char ** argv)
{
	if(args==3)
	{
		cFile file(argv[1],argv[2]);
		cout<<file.filename()<<endl;
	}
	else
		fprintf(stderr,"usage:%s who .ext\n",argv[0]);

}

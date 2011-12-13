#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/timeb.h>
#include <sys/file.h> //flock
#include "misc.h"
#include <errno.h>
#include <string>
#include <unistd.h>
using namespace std;
#define DEBUG_OUTPUT 1
#include "ctype.h"

void StrToLower(const char * sStr, int nLen)
{
    register char * beg=(char*)sStr;
    if (nLen==0)
        nLen=strlen(sStr);
    register char * end=(char*)sStr+nLen;

    while (beg<end && *beg)
    {
        *beg=tolower(((unsigned char)(*beg)));
        ++beg;
    }

}
int isuserid(const char *uid)
{
	int len = strlen(uid);
	if(len)
	{
		int i;
		for(i = 0 ; i < len ; i ++)
		{
			if(!((uid[i] >= 0x30 && uid[i] <= 0x39) || (uid[i] >= 0x61 && uid[i] <= 0x7a) || (uid[i] >= 0x41 && uid[i] <= 0x5a) || uid[i] == '@' || uid[i] == '-' || uid[i] == '_' || uid[i] == '.')  )
				return 0;
		}
	}
	else return 0;
	return 1;
}
void debug_output(FILE *std,char *text,...)
{
        va_list ap;
        va_start(ap,text);
        if(std && DEBUG_OUTPUT)
        {
                char time[128];
                fprintf(std,"%s->",strtime(time,sizeof(time)));
                vfprintf(std,text,ap);
        }
        va_end(ap);
}

char *strtime(char *str,int strlen)
{
        struct tm tt ;
        time_t tms = time(NULL);
        struct tm *ttp = localtime_r(&tms,&tt);
        strftime(str,strlen,"%Y%m%d%H%M%S",ttp);
        return str;
}
char *long2time(long time,char *str,int strlen)
{
	time_t tms = time;
        struct tm tt ;
        struct tm *ttp = localtime_r(&tms,&tt);
        strftime(str,strlen,"%Y%m%d%H%M%S",ttp);
	return str;
}

long time2long(const char *str)
{
        struct tm tt ;
        char *p;
	p = strptime(str,"%Y%m%d%H%M%S",&tt);
	time_t t = mktime(&tt);
	return t;
}


void 	Hex2Str(const char *src,int len,char *dest)
{
	for(int i = 0 ; i < len ; i++)
		sprintf(dest + i*2,"%.2x",(unsigned char)src[i]);
}

// -- cSimpleListBase --------------------------------------------------------------

cSimpleListBase::cSimpleListBase(void)
{
  first=last=0; count=0;
}

cSimpleListBase::~cSimpleListBase()
{
  Clear();
}

void cSimpleListBase::Add(cSimpleItem *Item, cSimpleItem *After)
{
  if(After) {
    Item->next=After->next;
    Item->prev=After;
    After->next->prev = Item;
    After->next=Item;
    }
  else {
    Item->next=0;
    Item->prev = last;
    if(last) last->next=Item;
    else first=Item;
    }
  if(!Item->next) last=Item;
  count++;
}

void cSimpleListBase::Ins(cSimpleItem *Item)
{
  Item->next=first;
  if(first)first->prev = Item;
  first=Item;
  Item->prev = 0;
  if(!Item->next) last=Item;
  count++;
}

void cSimpleListBase::Del(cSimpleItem *Item, bool Del)
{
  if(first==Item) {
    first=Item->next;
    if(!first) last=0;
    else
    	first->prev=0;
    }
  else {
    cSimpleItem *item=first;
    while(item) {
      if(item->next==Item) {
        item->next=Item->next;
        if(!item->next) last=item;
	else
		item->next->prev = item;
        break;
        }
      item=item->next;
      }
    }
  count--;
  if(Del) delete Item;
}

void cSimpleListBase::Clear(void)
{
  while(first) Del(first);
  first=last=0; count=0;
}

cFile::cFile(const char *id,const char *fext,bool inhome):userid(id),fext(fext)
{
	char buff[1024];
	char ID[32];
	ID[0] = 0;
	int x1 = 0;
	char *pp = strchr(id,'@');
        int pi = pp?pp-id:32;
        strncat(ID,id,pi);
	int idlen = strlen(ID);
	for(x1 = 0 ; x1 < idlen; x1 ++)
		if(ID[x1]=='.' ||  ID[x1]=='/' ||ID[x1]=='\\')ID[x1]='_';

	strcpy(buff, "/opt/usr/aa/aa");
	buff[9] =  ID[idlen - 2];
	buff[10] = ID[idlen - 1];
	buff[12] = ID[idlen - 4];
	buff[13] = ID[idlen - 3];

	//strncat (buff,ID,32);//opt/usr/aa/aa/UserID
	if(inhome)
	{
		dir = buff;
		strcat (buff,"/");//opt/usr/aa/aa/UserID/
		strncat (buff,id,32);//opt/usr/aa/aa/UserID/UserID
	}
	strcat (buff,fext);
	filepath = buff;
	fd = NULL;
}
bool	cFile::reopen(const char *type)
{
	close();
	return open(type);
}
bool	cFile::open(const char *type)
{

	fd = fopen(filepath.c_str(),type);
	if(!fd && type[0]!='r')
	{
		char tpath[1024];
		strncpy(tpath,filepath.c_str(),sizeof(tpath));
		char *p = tpath + 9;
		while(p=strchr(p,'/'))
		{
			*p = 0;
			mkdir(tpath,0777);
			//printf("mkdir:%s\n",tpath);
			*p = '/';
			p++;
		}
		fd = fopen(filepath.c_str(),type);
	}
	if(fd)
		if(type[0] == 'w' || type[0] == 'a' || type[1] == '+')
		{
			flock(fileno(fd),LOCK_EX);
		}
	//printf("%s,%s,%s\n",filepath.c_str(),type,fd?"OK":"FALSE");
	return fd?true:false;
}
void cFile::close()
{
	if(fd)
	{
		//printf("close %s\n",filepath.c_str());
		fclose(fd);
		fd=NULL;
	}
}
void cFile::funlink(bool home)
{
	close();
	unlink(filepath.c_str());
	if(home)
		rmdir(dir.c_str());
		
}

cFile::~cFile()
{
	close();
}
bool	cFile::exist()
{
	struct stat sb;
	int a = !stat(filepath.c_str(),&sb);
	printf("%s,%d\n",filepath.c_str(),a);
	if(a)
		printf("ok\n");
	else
		printf("not ok\n");
	return a;
}
size_t cFile::read(void*buff, size_t n, size_t s)
{
	return fread(buff,n,s,fd);
}
size_t cFile::write(void*buff, size_t n, size_t s)
{
	return fwrite(buff,n,s,fd);
}
void	cFile::seek(long offset, int whence)
{
	fseek(fd,offset,whence);
}

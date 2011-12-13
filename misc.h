#ifndef _USERV_MISC
#define _USERV_MISC
#include <stdlib.h>
#include <stdio.h>
int checkHomeFile(const char * ID  , const char * fext);
FILE * userHomeFile(const char * ID  ,  const char * fext , char * type , char * fBuff);
int formatUserID(char *str);
char *strtime(char *str,int strlen);
void debug_output(FILE *std,char *text,...);
int exceptions(char *err);
void    Hex2Str(const char *src,int len,char *dest);
long time2long(const char *str);
char *long2time(long time,char *str,int strlen);
void debug_output(FILE *std,char *text,...);
int isuserid(const char *uid);
void StrToLower(const char * sStr, int nLen);
// ----------------------------------------------------------------

class cSimpleListBase;

class cSimpleItem {
friend class cSimpleListBase;
private:
  cSimpleItem *next;
  cSimpleItem *prev;
public:
  virtual ~cSimpleItem() {}
  cSimpleItem *Next(void) const { return next; }
  cSimpleItem *Prev(void) const { return prev; }
  };

class cSimpleListBase {
protected:
  cSimpleItem *first, *last;
  int count;
public:
  cSimpleListBase(void);
  ~cSimpleListBase();
  void Add(cSimpleItem *Item, cSimpleItem *After=0);
  void Ins(cSimpleItem *Item);
  void Del(cSimpleItem *Item, bool Del=true);
  void Clear(void);
  int Count(void) const { return count; }
  };

template<class T> class cSimpleList : public cSimpleListBase {
public:
  T *First(void) const { return (T *)first; }
  T *Last(void) const { return (T *)last; }
  T *Next(const T *item) const { return (T *)item->cSimpleItem::Next(); }
  T *Prev(const T *item) const { return (T *)item->cSimpleItem::Prev(); }
  };

#include <string>
using namespace std;

class cFile
{
	private:
		FILE *fd;
		string	filepath;
		string  dir;
		string 	userid;
		string 	fext;
	public:
		cFile(const char *id,const char *fext,bool inhome=false);
		~cFile();
		bool	open(const char *type);
		void close();
		bool	exist();
		void	funlink(bool home=false);
		size_t read(void*buff, size_t n, size_t s);
		size_t write(void*buff, size_t n, size_t s);
		bool	reopen(const char *type);
		void	seek(long offset, int whence);
		FILE 	*handle(){return fd;};
		string const &filename()const {return filepath;};
};
#endif

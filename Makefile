all : lserv
FLAGS=-g -O2 -Wall
DEF=-D_GNU_SOURCE 
#-DDEBUGUDP 

#-DGDB -DDEBUGUDP 

lserv : main.c ddap.c ddap.o service.o misc.o
	g++ $(DEF) $(FLAGS) -o lserv main.c ddap.o service.o misc.o -pthread

homedir: homedir.cpp misc.o
	g++ $(DEF) $(FLAGS) -o homedir homedir.cpp misc.o -pthread

ddap.o : ddap.c 
	g++ $(DEF) $(FLAGS) -c ddap.c 

service.o : service.cpp
	g++ $(FLAGS) -c service.cpp

misc.o : misc.cpp
	g++ $(FLAGS) -c misc.cpp

clean : 
	rm -f *.o
	rm -f lserv
	rm -f homedir
	rm -f *.log
	rm -f *~

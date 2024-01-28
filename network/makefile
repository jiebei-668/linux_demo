# 在make前需要单独制作动态库和静态库 libsocket.so libsocket.a 命令在最后
all: demo01 demo02 demo03 demo04 demo05 demo06 demo07 demo08 
demo01: demo01.c
	gcc -g -o demo01 demo01.c
demo02: demo02.c
	gcc -g -o demo02 demo02.c
demo03: demo03.cpp
	g++ -g -o demo03 demo03.cpp -lpthread
demo04: demo04.cpp
	g++ -g -o demo04 demo04.cpp
demo05: demo05.cpp
	g++ -g -o demo05 demo05.cpp
demo06: demo06.cpp
	g++ -g -o demo06 demo06.cpp
demo07: demo07.cpp
	g++ -g -o demo07 demo07.cpp
demo08: demo08.cpp
	g++ -g -o demo08 demo08.cpp -L./ -lsocket
libsocket.a: libsocket.cpp libsocket.h
	g++ -c -o libsocket.o libsocket.cpp
	ar rsv libsocket.a libsocket.o
	rm -f libsocket.o
libsocket.so: libsocket.cpp libsocket.h
	g++ -c -fpic -o libsocket.o libsocket.cpp
	g++ -shared -o libsocket.so libsocket.o
	rm -f libsocket.o
clean:
	rm -rf bin/*
	rm -rf libsocket.so libsocket.a demo01 demo02 demo03 demo04 demo05 demo06 demo07 demo08

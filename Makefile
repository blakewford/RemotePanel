CC=g++
FLAGS=-O2 -std=c++11

remote: main.cpp libhost.a libremote.a
	$(CC) -L. $(FLAGS) main.cpp -o $@ -pthread -Wno-pointer-arith -lhost -lSDL2

libhost.a: client.cpp server.cpp
	$(CC) -I/usr/include/SDL2 $(FLAGS) client.cpp server.cpp -c -Wno-pointer-arith
	ar rvs $@ client.o server.o

libremote.a: client.cpp
	$(CC) $(FLAGS) client.cpp -c -Wno-pointer-arith
	ar rvs $@ client.o

clean:
	-@rm remote
	-@rm libhost.a
	-@rm libremote.a
	-@rm *.o

CC=g++
FLAGS=-O2 -std=c++11

remote: main.cpp libremote.a
	$(CC) -L. $(FLAGS) main.cpp -o $@ -pthread -Wno-pointer-arith -lremote -lSDL2

libremote.a: client.cpp server.cpp
	$(CC) -I/usr/include/SDL2 $(FLAGS) client.cpp server.cpp -c -Wno-pointer-arith
	ar rvs $@ client.o server.o

clean:
	-@rm remote
	-@rm libremote.a
	-@rm *.o

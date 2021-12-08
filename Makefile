CC=g++
FLAGS=-O2 -std=c++11
ifeq ("$(TERM_PROGRAM)","Apple_Terminal")
INCLUDE=/Library/Frameworks/SDL2.framework/Headers
LIBS=-F/Library/Frameworks -framework SDL2
else
INCLUDE=/usr/include/SDL2
LIBS=-lSDL2
endif

remote: main.cpp libhost.a libremote.a
	$(CC) -L. $(FLAGS) main.cpp -o $@ -pthread -lremote -lhost $(LIBS)

libhost.a: server.cpp
	$(CC) -I$(INCLUDE) $(FLAGS) server.cpp -c
	ar rvs $@ server.o

libremote.a: client.cpp nostdlib.cpp
	$(CC) $(FLAGS) client.cpp nostdlib.cpp -c
	ar rvs $@ client.o nostdlib.o

clean:
	-@rm remote
	-@rm libhost.a
	-@rm libremote.a
	-@rm *.o

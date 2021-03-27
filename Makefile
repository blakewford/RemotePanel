CC=g++
FLAGS=-O2 -std=c++11

remote: main.cpp libremote.a
	$(CC) -L. $(FLAGS) main.cpp -o $@ -pthread -Wno-pointer-arith -lremote

libremote.a: client.cpp
	$(CC) $(FLAGS) $< -c -Wno-pointer-arith
	ar rvs $@ client.o

clean:
	-@rm remote
	-@rm libremote.a
	-@rm *.o

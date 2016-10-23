all:
	gcc server.c -o server -lpthread
	
server:
	gcc server.c -o server -lpthread


clean:
	rm server

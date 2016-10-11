all:
	gcc server.c -o server -lpthread
	gcc cli/client.c -o cli/client
server:
	gcc server.c -o server -lpthread
client:
	gcc cli/client.c -o server

clean:
	rm server
	rm cli/client
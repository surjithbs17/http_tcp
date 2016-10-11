/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr


#define MAX_LINE 200
 
int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    //char message[1000] , server_reply[2000];
    char *server_reply = (char *)malloc(sizeof(char)*(MAX_LINE+1));
    char *message = (char *)malloc(sizeof(char)*(MAX_LINE+1));
    bzero(server_reply,sizeof(server_reply));
    bzero(message,sizeof(message));

    printf("Size  - %d",sizeof(message));
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 80 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
     
    //keep communicating with server
    while(1)
    {
        bzero(message,sizeof(message));
     
        printf("Enter message : ");
        scanf("%s" , message);
        printf("%s - message         Size - %d\n",message,strlen(message));
         
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }
         
        printf("Server reply : %s\n",server_reply );

        bzero(server_reply,strlen(server_reply));
    }
     
    close(sock);
    return 0;
}
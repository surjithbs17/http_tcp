
#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <fcntl.h>
#include <netdb.h>


#define MAX_LINE 2000
#define MESSAGE_LENGTH 10000
#define BYTES 1024



char *root_dir; 

int socket_creation(char* port)
{
    struct addrinfo server;
    struct addrinfo *res, *p;
    int socket_desc;


    memset (&server, 0, sizeof(server));
    // getaddrinfo for host
    server.ai_family = AF_INET;
    server.ai_socktype = SOCK_STREAM;
    server.ai_flags = AI_PASSIVE;

    if (getaddrinfo( NULL, port, &server, &res) != 0)
    {
        printf("get address error");
        exit(1);
    }

    for (p = res; p!=NULL; p=p->ai_next)
    {
        socket_desc = socket(p->ai_family, p->ai_socktype, 0);
        if (socket_desc == -1) continue;
        if (bind(socket_desc, p->ai_addr, p->ai_addrlen) == 0) break;
        printf("count this! \n");
    }

    if (p==NULL)
    {
        printf ("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (socket_desc, 50) != 0 )
    {
        printf("listen error");
        exit(1);
    }

    return socket_desc;

}


//client connection
void *connection_handler(int sock_client)
{
    
    char recv_buf[MESSAGE_LENGTH], *http_cmd[3], data_to_send[BYTES], path_to_file[MESSAGE_LENGTH];

    int s, file, bytes_read;

    bzero(recv_buf,sizeof(recv_buf));

    s = recv(sock_client, recv_buf, MESSAGE_LENGTH, 0);

    if( s < 0 )
    {    
        printf("Recieve error\n");
    }
    else if (s == 0)
    { 
        printf("Client disconnected (recv returns zero).\n");
    }
    else
    {
        printf("%s", recv_buf);
        http_cmd[0] = strtok (recv_buf, " \t\n");
        if ( strncmp(http_cmd[0], "GET\0", 4)==0 )
        {
            http_cmd[1] = strtok (NULL, " \t");
            http_cmd[2] = strtok (NULL, " \t\n");
            
            int http_1_0 = strncmp( http_cmd[2], "HTTP/1.0", 8);
            int http_1_1 = strncmp( http_cmd[2], "HTTP/1.1", 8);

            if ( http_1_0!=0 && http_1_1!=0 )
            {
                write(sock_client, "HTTP/1.0 400 Bad Request\n", 25);
            }
            else
            {
                int check_default =strncmp(http_cmd[1], "/\0", 2);

                if ( check_default ==0 )
                {    
                    http_cmd[1] = "/index.html";        
                }

                strcpy(path_to_file, root_dir);

                printf("Path - %s\n",path_to_file);
                strcpy(&path_to_file[strlen(root_dir)], http_cmd[1]);
                
                printf("file: %s\n", path_to_file);

                if ( (file=open(path_to_file, O_RDONLY))!=-1 )    //FILE FOUND
                {
                    send(sock_client, "HTTP/1.0 200 OK\n\n", 17, 0);
                    while ( (bytes_read=read(file, data_to_send, BYTES))>0 )
                    {    
                        write (sock_client, data_to_send, bytes_read);
                    }
                }
                else    write(sock_client, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
            }
        }
    }

    //Closing SOCKET
    shutdown (sock_client, SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(sock_client);
    sock_client=-1;
}
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
    char port[6];
    root_dir = getenv("PWD"); //getting the root directory and storing it in variable ROOT
    strcpy(port,"10000");

    socket_desc = socket_creation(port);
    int optval = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

    
    c = sizeof(struct sockaddr_in); 

    while(1)
    {
        
        int len = sizeof(client);

        if((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&len)) < 0)
        {
            puts("Connection Error");
            break;    
        }
        
         
        pthread_t sniffer_thread;
        

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */



 /*
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    //char *message , client_message[2000];
    
    char *client_message = (char *)malloc(sizeof(char)*(MAX_LINE+1));
    char *message = (char *)malloc(sizeof(char)*(MAX_LINE+1));
    bzero(client_message,sizeof(client_message));
    bzero(message,sizeof(message));

    //Send some messages to the client
   /* message = "\nGreetings! I am your connection handler\n";
    write(sock , message , strlen(message));
     
    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));
     */
    //Receive a message from client
/*
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        printf("Client Message : %s   Size - %d \n", client_message,strlen(client_message));
        write(sock , client_message , strlen(client_message));
        bzero(client_message,strlen(client_message));
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //Free the socket pointer
    free(socket_desc);
     
    return 0;
}

*/

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


char* data_finder(char* format)
{
    int port,content_type;
    char parsed_string[10000];
    char line_parsed[200];
    char* content_parsed = malloc(200);
    char* content_parsed_1 = malloc(200);
    char* root_string = malloc(200);
    char* token;
    token = "1";


    //printf("%s\n",format );
    char data[100];
    strcpy(data,".");
    strcpy(&data[1],format);


    //printf("%s\n",data );

    FILE *f;
    bzero(content_parsed,sizeof(content_parsed));
    //bzero(content_parsed[1],sizeof(content_parsed[1]));   
    f = fopen("ws.conf", "r");

    port = strcmp(format,"port");
    if(port == 0)
    {
        strcpy(data,"Listen");
    }

    int root = strcmp(format,"root");
    if(root == 0)
    {
        strcpy(data,"DocumentRoot");
    }



    while(token != NULL)
    {
        token = fgets(parsed_string,10000,f);
        //printf(" -- %s",parsed_string);
        if(parsed_string[0] == '#')
        {   
            //printf("It is a comment \n");
            continue;
        }
        else
        {
            sscanf(parsed_string,"%s %s",content_parsed,content_parsed_1);
            //printf("Parsed 1 \n\n%s\nParsed 2 \n\n%s\n",content_parsed,content_parsed_1);
            int comp = strcmp(content_parsed,data);
            if(comp == 0)
            {
                if(root == 0)
                {
                    //sscanf(content_parsed_1,"\"%s \"",root_string);
                    root_string = strtok(content_parsed_1,"\"");
                    strcpy(content_parsed_1,root_string);

                }   
                return content_parsed_1;
                break;
            }



        }
    }


}


char* find_the_file_format(char *entire_path)
{
    
    char *token_string = malloc(1000);
    char *recent_token = malloc(1000);
    char *recent_token_format = malloc(1000);
    char *latest_token = malloc(1000);
    bzero(token_string,sizeof(token_string));
    const char s[2] = "/";
    
    token_string = strtok(entire_path, s);

    while( token_string != NULL ) 
    {
        //printf( "%s\n", token_string );
    
      //memset (token_string, 0, sizeof(token_string));
        token_string = strtok(NULL, "/");
        if(token_string != NULL)
        {
            strcpy(recent_token,token_string);
        }
      //printf("%s\n",token_string );
    }

    recent_token_format = strtok(recent_token, "."); 
    while(recent_token_format != NULL)
    { 
        recent_token_format = strtok(NULL, ".");
        
        if(recent_token_format != NULL)
        {
            strcpy(latest_token,recent_token_format);
        }
    }

    //printf("\nToken -- %s\n%s\n",recent_token,recent_token_format);
    return latest_token;
}


char* content_type(char* path)
{
    
    //printf("%s\n",path );
    char* type = find_the_file_format(path);
    char* type_string = data_finder(type);
    char* final_string = malloc(200);
    
    //printf("%s\n",type_string );
    strcpy(final_string,"Content-Type: ");

    //printf("%s\n",final_string );
    strcpy(final_string+14,type_string);
    strcpy(final_string+(strlen(final_string)),"\n\n");


    //printf("%s\n",final_string );

    return final_string;
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
                    send(sock_client, "HTTP/1.0 200 OK\n", 16, 0);

                    char* content_string = content_type(path_to_file);
                    printf("\n\n\n\n\nContent String%s\n",content_string);
                    send(sock_client, content_string, strlen(content_string),0);
                    while ( (bytes_read=read(file, data_to_send, BYTES))>0 )
                    {    
                        write (sock_client, data_to_send, bytes_read);
                        printf("%s\n",data_to_send);
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
     
    char* port;
    root_dir = data_finder("root"); //getting the root directory and storing it in variable ROOT


    //strcpy(port,"10000");
    port = data_finder("port");
    socket_desc = socket_creation(port);
    int optval = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

    
     
    int len = sizeof(client);

    while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&len)) > 0)
    {
        
                 
        pthread_t sniffer_thread;
        
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , client_sock) < 0)
        {
            perror("could not create thread");
            exit(1);
        }
         
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
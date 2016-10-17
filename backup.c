
#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>


#define MAX_LINE 2000
#define MESSAGE_LENGTH 10000
#define BYTES 1024

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


#define METHOD 0
#define URL 1
#define VERSION 2
#define HTTP_1_0 0
#define HTTP_1_1 1

char *root_dir; 

struct socket_info {
    int parent_socket;
    int child_socket;
};

struct data_for_tit {
    char* buffer;
    int child_socket;
};


char* error_handling(int err_no,int reason_no,int version_no,char* arg, int socket)
{

    char* version[2]; 
    char* reason[3];
    char* error_string = malloc(MESSAGE_LENGTH);
    char* http_string = malloc(MESSAGE_LENGTH);
    reason[0] = "Method";
    reason[1] = "URL";
    reason[2] = "HTTP-Version";
    version[0] = "HTTP/1.0";
    version[1] = "HTTP/1.1";
    printf("Passed 2\n");
    bzero(error_string,sizeof(error_string));
    bzero(http_string,sizeof(http_string));


    switch(err_no)
    {
        case 400:
        {
            //printf("Passed 3\n");
            sprintf(http_string,"%s 400 Bad Request\r\n",version[version_no]);
            //printf("Passed 4\n");
            write(socket, http_string, 26);
            sprintf(error_string,"<!DOCTYPE html>\r\n<html><body> 400 Bad Request Reason: Invalid %s %s</body></html>\r\n",reason[reason_no],arg);
            write(socket,error_string,strlen(error_string)+1);
            printf("Error code 400!\n");

            break;
        }
        case 404:
        {
            sprintf(http_string,"%s 404 Not Found\r\n",version[version_no]);
            write(socket, http_string, 24);
            sprintf(error_string,"<!DOCTYPE html>\r\n<html><body>404 Not Found Reason URL does not exist: Invalid  %s %s</body></html>\r\n",reason[reason_no],arg);
            write(socket,error_string,strlen(error_string)+1);
            printf("Error code 404!\n");
            break;
        }
        case 501:
        {
            sprintf(http_string,"%s 501 Not Implemented\r\n",version[version_no]);
            write(socket, http_string, 30);
            sprintf(error_string,"<!DOCTYPE html>\r\n<html><body> Not Implemented: Invalid File %s</body></html>\r\n",arg);
            write(socket,error_string,strlen(error_string)+1);
            printf("Error code 501!\n");
            break;
        }

        case 500:
        {
            sprintf(http_string,"%s 500 Internal Server Error: cannot allocate memory\r\n",version[version_no]);
            write(socket, http_string, 60);
            printf("Error code 500!\n");
            write(socket,error_string,strlen(error_string)+1);
            break;
        }

        default:
        {

            write(socket, "HTTP/1.1 400 Bad Request\n", 25);
            sprintf(error_string,"<html><body> 400  Bad Request Reason: Invalid %s %s</body></html>\n",reason[reason_no],arg);
            write(socket,error_string,strlen(error_string)+1);
            printf("Error code Default!\n");
        }
    }

    printf("Error Handled!\n");

}

char* content_length(char* path)
{
    char* final_string = malloc(200);
    char* length_string = malloc(200);
    bzero(final_string,sizeof(final_string));
    bzero(length_string,sizeof(length_string));
    strcpy(final_string,"Content-Length: ");
    
 
    FILE *get_file = fopen(path,"r+");
    if(get_file == NULL)
    {
        printf("\nError: File open %s\n",path);
    }

    fseek(get_file,0,SEEK_END);  //Routing to check the file size
    long filesize = ftell(get_file);
    rewind(get_file);
    fclose(get_file);

    sprintf(length_string,"%lld\r\n",filesize);
    strcpy(&final_string[16],length_string);
    return final_string;
}

int doesFileExist(const char *filename) 
{
    struct stat st;
    int result = stat(filename, &st);
    if(result == 0)
    {
        printf("\nfile does exist!\n");
        return 0;
    }
    else
        return -1;
}


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

    printf("Port %s\n",port );
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

    int alive = strcmp(format,"KeepaliveTime");
    if(alive == 0)
    {
        strcpy(data,"KeepaliveTime");
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




void *thread_inside_thread(void* data_arg)
{
    
    char recv_buf[MESSAGE_LENGTH], *http_cmd[3], *alive[2],data_to_send[BYTES], path_to_file[MESSAGE_LENGTH];
    const char keep_alive[MESSAGE_LENGTH];
    int s, file, bytes_read;
    

    bzero(recv_buf,sizeof(recv_buf));
    bzero(keep_alive,sizeof(keep_alive));
    struct data_for_tit* data = (struct data_for_tit*)data_arg ;

    printf("Recieved string from struct - %s \n\n",data->buffer);
    //strcpy(recv_buf,data_arg->recv_buf);
    strcpy(&recv_buf,data->buffer);
    bzero(data->buffer,sizeof(data->buffer));
    int sock_client  = data->child_socket;
    

        printf("%s", recv_buf);
        http_cmd[0] = strtok (recv_buf, " \t\n");
        if ( strncmp(http_cmd[0], "GET\0", 4)==0 )
        {
            http_cmd[1] = strtok (NULL, " \t");
            http_cmd[2] = strtok (NULL, " \t\n");
            
            //int http_1_0 = strncmp( http_cmd[2], "HTTP/1.0", 8);
            int http_1_1 = strncmp( http_cmd[2], "HTTP/1.1", 8);

            const char needle[50] = "Connection: keep-alive";
            char *ret;


            strcpy(&keep_alive,data->buffer);
            printf(RED "Strcpy %s \n\n\n\n" RESET, keep_alive );

            ret = strstr(keep_alive, needle);
            
            if(ret!=NULL)
            {
                struct timeval tv;
                tv.tv_sec = 30;       /* Timeout in seconds */
                setsockopt(sock_client, SOL_SOCKET, SO_SNDTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
                setsockopt(sock_client, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
            }


            if (http_1_1!=0 )
            {
                //write(sock_client, "HTTP/1.1 400 Bad Request\n", 25);
                error_handling(400,METHOD,HTTP_1_1,http_cmd[2],sock_client);
            }
            else
            {
                int check_default =strncmp(http_cmd[1], "/\0", 2);

                if ( check_default ==0 )
                {    
                    http_cmd[1] = "/index.html";        
                }

                strcpy(path_to_file, root_dir);

                //printf("Path - %s\n",path_to_file);

                strcpy(&path_to_file[strlen(root_dir)], http_cmd[1]);
                
                //printf("file: %s\n", path_to_file);

                if ( (file=open(path_to_file, O_RDONLY))!=-1 )    //FILE FOUND
                {
                    send(sock_client, "HTTP/1.1 200 OK\n", 16, 0);

                    char* content_string = content_type(path_to_file);
                    char* content_len = content_length(path_to_file);
                    //printf("\n\n\n\n\nContent String%s\n",content_string);
                    send(sock_client, content_string, strlen(content_string),0);
                    while ( (bytes_read=read(file, data_to_send, BYTES))>0 )
                    {    
                        write (sock_client, data_to_send, bytes_read);
                        //printf("%s\n",data_to_send);
                    }
                    printf(GRN"Out of File Sent While Loop\n"RESET);
                }
                else    
                {
                    printf("File Not found\n");
                    //write(sock_client, "HTTP/1.1 404 Not Found\n", 23); //FILE NOT FOUND
                    error_handling(404,METHOD,HTTP_1_1,http_cmd[1],sock_client);

                }

            }
            printf(RED "\n\n-------------Completed the thread ----------------" RESET);
        }
}



//client connection
void *connection_handler(int sock_client)
{
    
    char recv_buf[MESSAGE_LENGTH],recv_buf_1_1[MESSAGE_LENGTH], *http_cmd[3], data_to_send[BYTES], path_to_file[MESSAGE_LENGTH];

    int s, file, bytes_read;

    bzero(recv_buf,sizeof(recv_buf));

    /*
    struct socket_info* sock_info_in_child = (struct socket_info*)sock_info_argument ;

    int socket_to_close = sock_info_in_child->parent_socket;
    int sock_client = sock_info_in_child->child_socket;

    printf("Sockeet to close - %d\n Socket to survive - %d \n",socket_to_close,sock_client );

    close(socket_to_close);
    */
    printf(YEL"In First Thread --- %d\n\n\n" RESET,sock_client);

    //struct timeval tv;
 
    //tv.tv_sec = 30;       /* Timeout in seconds */
 
    //setsockopt(sock_client, SOL_SOCKET, SO_SNDTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
    //setsockopt(sock_client, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
    //setsockopt(sock_client, SOL_TCP, TCP_USER_TIMEOUT,(struct timeval *)&tv,sizeof(struct timeval));
    
    //while(1)
    //{
    //while((
    s = recv(sock_client, recv_buf, MESSAGE_LENGTH, 0);
    //{
    printf(YEL"Accepting in slave\n\n"RESET);


    if( s < 0 )
    {    
        printf("Recieve error\n");
    }
    else if (s == 0)
    { 
        printf(YEL"Client disconnected (recv returns zero).\n"RESET);
    }
    else
    {
        strcpy(&recv_buf_1_1,&recv_buf);
/*        printf("\n\n\n\n\nCreating a thread\n");
        if( pthread_create( &second_thread , NULL ,  thread_inside_thread , data) < 0)
        {
            perror("could not create thread");
            exit(1);
        }
*/
        //printf(YEL"%s"RESET, recv_buf);
        http_cmd[0] = strtok (recv_buf, " \t\n");
        if ( strncmp(http_cmd[0], "GET\0", 4)==0 )
        {
            http_cmd[1] = strtok (NULL, " \t");
            http_cmd[2] = strtok (NULL, " \t\n");
            
            int http_1_0 = strncmp( http_cmd[2], "HTTP/1.0", 8);
            int http_1_1 = strncmp( http_cmd[2], "HTTP/1.1", 8);

            if ( http_1_0!=0 && http_1_1!=0 )
            {
                //write(sock_client, "HTTP/1.0 400 Bad Request\n", 25);
                error_handling(400,METHOD,HTTP_1_1,http_cmd[2],sock_client);
            }
            else if (http_1_0 == 0)
            {
                int check_default =strncmp(http_cmd[1], "/\0", 2);

                if ( check_default ==0 )
                {    
                    http_cmd[1] = "/index.html";        
                }

                strcpy(path_to_file, root_dir);

                printf(YEL"Path - %s\n"RESET,path_to_file);

                strcpy(&path_to_file[strlen(root_dir)], http_cmd[1]);
                
                printf(YEL"file: %s\n"RESET, path_to_file);

                if ( (file=open(path_to_file, O_RDONLY))!=-1 )    //FILE FOUND
                {
                    send(sock_client, "HTTP/1.0 200 OK\n", 16, 0);

                    char* content_string = content_type(path_to_file);
                    //printf("\n\n\n\n\nContent String%s\n",content_string);
                    send(sock_client, content_string, strlen(content_string),0);
                    while ( (bytes_read=read(file, data_to_send, BYTES))>0 )
                    {    
                        write (sock_client, data_to_send, bytes_read);
                        //printf("%s\n",data_to_send);
                    }
                }
                else
                {
                    printf("File Not found\n");
                    //write(sock_client, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                    error_handling(404,METHOD,HTTP_1_0,http_cmd[1],sock_client);
                }
            }
            else if(http_1_1 == 0)
            {
                //pthread_t second_thread;
                //struct data_for_tit *data = malloc(sizeof(struct data_for_tit));
        
                //data->buffer = recv_buf;
                //data->child_socket = sock_client;


                pthread_t second_thread;
                struct data_for_tit *data = malloc(sizeof(struct data_for_tit));

                data->buffer = recv_buf_1_1;
                data->child_socket = sock_client;

                printf(YEL"\nCreating a thread \n"RESET);
                if( pthread_create( &second_thread , NULL ,  thread_inside_thread , data) < 0)
                {
                    perror("could not create thread");
                    exit(1);
                }


                while((s = recv(sock_client, recv_buf, MESSAGE_LENGTH, 0)) > 0)
                {  

                    if (s == 0)
                    { 
                        printf("Client disconnected (recv returns zero).\n");
                    }
                    else
                    {
                        pthread_t third_thread;
                        struct data_for_tit *data = malloc(sizeof(struct data_for_tit));
        
                        data->buffer = recv_buf;
                        data->child_socket = sock_client;
                        printf(YEL"\nCreating a thread_ 2nd \n"RESET);
                        if( pthread_create( &second_thread , NULL ,  thread_inside_thread , data) < 0)
                        {
                            perror("could not create thread");
                            exit(1);
                        }

                    }

                }
                if( s < 0 )
                {    
                        printf("Recieve error\n");
                }

            }
        }
    
    }
    //}
    //Closing SOCKET
    printf(GRN "\n\n\nOut of while loop\n\n" RESET);
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

    char* alive_time = data_finder("KeepaliveTime");
    int time_to_live = atoi(alive_time);

    printf("\n Time to live - %d",time_to_live);
    //strcpy(port,"10000");
    port = data_finder("port");
    socket_desc = socket_creation(port);
    int optval = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

    struct socket_info *sock_info = malloc(sizeof(struct socket_info));

    
     
    int len = sizeof(client);

    while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&len)) > 0)
    {
        //sock_info->parent_socket = socket_desc;
        //sock_info->child_socket = client_sock;
        printf(BLU"Socket_DESC --- %d\nClient_Sock --- %d\n\n\n"RESET,socket_desc,client_sock);


        //printf(" Master Sockeet to Survive - %d\n Socket to close - %d \n",sock_info->parent_socket,sock_info->child_socket);
        
        printf(BLU"Accepting in Master\n\n"RESET);
        pthread_t first_thread;
        
        if( pthread_create( &first_thread , NULL ,  connection_handler , client_sock) < 0)
        {
            perror("could not create thread");
            exit(1);
        }
         
        //close(client_sock);
        //puts("Handler assigned");
        printf(BLU "Out of Main Accept While Loop\n" RESET);


    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    free(socket_desc);
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
#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <fcntl.h>
#include <sys/stat.h>
#define MESSAGE_LENGTH 500

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
			printf("Passed 3\n");
			sprintf(http_string,"%s 400 Bad Request\n",version[version_no]);
			//printf("Passed 4\n");
			send(socket, http_string, 25, 0);
			sprintf(error_string,"<html><body> 400	Bad	Request	Reason:	Invalid	%s %s</body></html>",reason[reason_no],arg);
			send(socket,error_string,strlen(error_string)+1,0);
			printf("Error code 400!\n");

			break;
		}
		case 404:
		{
			sprintf(http_string,"%s 404 Not Found\n",version[version_no]);
			send(socket, http_string, 23, 0);
			sprintf(error_string,"<html><body>404 Not Found	Reason URL does not exist: Invalid	%s %s</body></html>",reason[reason_no],arg);
			send(socket,error_string,strlen(error_string)+1,0);
			printf("Error code 404!\n");
			break;
		}
		case 501:
		{
			sprintf(http_string,"%s 501 Not Implemented\n",version[version_no]);
			send(socket, http_string, 29, 0);
			sprintf(error_string,"<html><body> Not Implemented: Invalid	File %s</body></html>",arg);
			send(socket,error_string,strlen(error_string)+1,0);
			printf("Error code 501!\n");
			break;
		}

		case 500:
		{
			sprintf(http_string,"%s 500 Internal Server	Error: cannot allocate memory\n",version[version_no]);
			send(socket, http_string, 59, 0);
			printf("Error code 500!\n");
			send(socket,error_string,strlen(error_string)+1,0);
			break;
		}

		default:
		{

			send(socket, "HTTP/1.1 400 Bad Request\n", 25, 0);
			sprintf(error_string,"<html><body> 400	Bad	Request	Reason:	Invalid	%s %s</body></html>",reason[reason_no],arg);
			send(socket,error_string,strlen(error_string)+1,0);
			printf("Error code Default!\n");
		}
	}

	printf("Error Handled!\n");

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

char* content_length(char* path)
{
	char* final_string = malloc(200);
	char* length_string = malloc(200);
	bzero(final_string,sizeof(final_string));
	bzero(length_string,sizeof(length_string));
	strcpy(final_string,"Content-Length: ");
	
	//struct stat st;
	//stat(path, &st);
	//long long int size = st.st_size;



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


	//printf("%s\n",final_string );

	return final_string;
}

void content_headers(char* path,int socket)
{
	char* content_string = content_type(path);
   	send(socket, content_string, strlen(content_string),0);
    char* content_length = content_length(path);
    send(socket, content_length, strlen(content_length),0);
}

int main()
{
	//char* value = data_finder(".html");
	/*
	char path[200] = "im/a/ges/exam.hel.lo.gif";
	
	char* format = find_the_file_format(path);
	//char* search = "DocumentRoot";
	printf("\n%s\n",format);

	char* value = data_finder("KeepaliveTime");
	printf("\n%s\n",value);

	strcpy(path,"im/a/ges/exam.gif");
	
	char* port = content_type(path);


	printf("%s\n",port);



	char* keepalive_string = strstr("Connection: ",recv_buf);
    printf(RED "Keep Aluve %s \n\n\n\n" RESET, keepalive_string );

	//printf("%s",config);
*/	char *path_to_file = malloc(400);
    bzero(path_to_file,sizeof(path_to_file));
    char req[200] = "/index.html";
    printf("Passed 1\n");
    //error_handling(400,1,0,req,5);
    char*  root_dir = data_finder("root");
     printf("Passed 1\n");

                strcpy(path_to_file, root_dir);
                 printf("Passed 1\n");

                printf("Path - %s\n",path_to_file);

                strcpy(&path_to_file[strlen(root_dir)], req);
                  printf("Path - %s\n",path_to_file);
                 printf("Passed 1\n");

    printf("%s\n",content_length(path_to_file));



}
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT "4343"
#define ENDWORD "endword"
int isSearch = 0;

//Menu & misc.:
int clientLoop(int servSock);
long getRequest();
long getPackLen(int servSock);
void consolePause();
void stdinFlush();
//Network
int sendRequest(int servSock);
int getServSock(char *hostname);
char *getResponse(int servSock);


/********************************************* MENU: *****************************************/

long getRequest()
{
	char buffer[256];
	long request = 0;
	//stdinFlush();
	while(1)
	{
		if( fgets(buffer, sizeof(buffer), stdin) )
		{
			if( sscanf(buffer, "%d", &request) == 1 )
				return request;
			else
				printf("Invalid input!\nMust be a number. Try again:\n");
		}
		else
			printf("Invalid input!\nMust be a number. Try again:\n");
	}
}

long getPackLen(int servSock)
{
	long len = 0;
	int bytesRead = 0;
	int n = 0;
	while(bytesRead < 4)
	{
		n = read(servSock, ((char*) &len) + bytesRead, 4 - bytesRead);
		if(n < 0)
			return n;
		bytesRead += n;
	}
	
	len = ntohl(len);
	return len;
}

char *getResponse(int servSock)
{
	char *response = NULL;
	int bytesDone = 0;
	int bytesLeft;
	int n;
	long len = getPackLen(servSock);
	if(len < 0)
		return NULL;
	bytesLeft = len;
	if( (response = (char*) malloc(len)) == NULL)
		return NULL;
	memset(response, '\0', len);
	while(bytesDone < len)
	{
		n = read(servSock, response+bytesDone, bytesLeft);
		if (n < 0)
        	return NULL;
        bytesDone += n;
        bytesLeft -= n;
    }
    
	return response;
}

int clientLoop(int servSock)
{
	char *response = NULL;
	long choice = 0;
	int rv;
	
	rv = sendRequest(servSock);
	if(rv < 0)
	{
		perror("Sending error");
		return 0;
	}
	
	response = getResponse(servSock);
	if(response == NULL)
	{
		perror("Error recieving");
		return 0;
	}
	if(!strcmp(response, ENDWORD))
	{
		free(response);
		return 0;
	}
	printf("%s\n", response);
	free(response);
	
	
	return rv;
}

void consolePause()
{
	//stdinFlush();
	printf("Enter anything to continue\n");
	//stdinFlush();
	
	getc(stdin);
}

void stdinFlush()
{
	int c;
	do
	{
		c = getchar();
	}
	while(c != '\n' && c != EOF);
}

/********************************** NETWORK *************************/

int sendRequest(int servSock)
{
	char *response = NULL;
	long request;
	request = getRequest();
	
	if(isSearch)
		isSearch = 0;
	else
	{
		if(request > 11 && request < 15)
			isSearch = 1;
	}
	
		
	request = htonl(request);
	if( write(servSock, &request, 4) != 4)
	{
		perror("Sending error");
		return -1;
	}
	
	return 1;
}

int getServSock(char *hostname)
{
	struct addrinfo hints;
	struct addrinfo *servinfo = NULL;
	struct addrinfo *p = NULL;
    int rv;
    int sockfd;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0)
	{
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }
	
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return -2;
    }
    freeaddrinfo(servinfo);
	return sockfd;
}

//***************************** [ MAIN ] *******************************//

int main(int argc, char *argv[])
{
	int servSock = 0;
	char *options = NULL;
	if(argc != 2)
	{
		fprintf(stderr,"Usage: %s hostname\n", argv[0]);
    	exit(0);
    }
	
	servSock = getServSock(argv[1]);
	if(servSock < 0)
	{
		perror("Error conneting");
		exit(1);
	}
	printf("Attempting connection...\n");
   	options = getResponse(servSock);
   	printf("Connection Established.\n");
	while(1)
	{
		if(!isSearch)
		{
			consolePause();
			printf("%s\n", options);
			printf("Enter your choice:\n");
		}
		if(clientLoop(servSock) == 0)
			break;
		
	}
	free(options);
	close(servSock);
	return 0;
}




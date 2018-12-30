/*
    Simple udp server
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
 
#define BUFLEN 512  //Max length of buffer
#define PORT 4445   //The port on which to listen for incoming data
 
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(void)
{
    struct sockaddr_in si_me, si_other;
     
    int s, i, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
    char msg[BUFLEN];
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

        printf("Waiting for request...\n");
        fflush(stdout);

        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom() : error!");
        }
        
	printf("Source : %s\n", inet_ntoa(si_other.sin_addr));
	printf("Port   : %d\n",ntohs(si_other.sin_port));	
	printf("Request: %s\n",buf);
	for(i = 0; i<40; i++)
	   printf("*");
	printf("\n");
        
	memset(buf,'\0', BUFLEN);
	
	for(;;){
	strcpy(buf,"0.1,0.2,0.3,0.4,0.5,0.6,0.7"); 
        if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
	
	if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            die("recvfrom()");
        }
	
	
	
	printf("Data sent: ");
        puts(buf);
	
	if(strcmp(buf,"off") == 0)
	  die("Terminated");
	
	}//end for	

    close(s);
    return 0;
}

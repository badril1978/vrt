/*
    Simple udp server
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <unistd.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>

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
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    int s, i, slen = sizeof(si_other) , recv_len, n;
    char buf[BUFLEN];
    fd_set rset;
     
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

      
   

    //keep listening for data
    while(1)
    {
        printf("Waiting for data...");
        fflush(stdout);
        FD_ZERO(&rset); 
	FD_SET(s, &rset);  
	
       
	n = select(s+1, &rset, 0, 0, &tv); 	
        //  n = select(s+1, &rset, 0, 0, 0); 	
 
	if(n < 0)
	  {
		perror("ERROR Server : select()\n");
		close(s);
		exit(1);
	  }

	if (FD_ISSET(s, &rset)) 
	{
	
		printf("Server is ready to read\n");

	        //try to receive some data, this is a blocking call
	        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
	        {
	            die("recvfrom()");
	        }
         
	        //print details of the client/peer and the data received
	        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
	        printf("Data: %s\n" , buf);
         
	        //now reply the client with the same data
	        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
	        {
	            die("sendto()");
	        }
		
		FD_CLR(s, &rset);
	 }
	 else
	 {
		printf("server ain't ready\n");
		sleep(1);
		
	 }
 
         memset(&buf, 0, sizeof(buf));
         bzero(buf, BUFLEN);
      
    }
    close(s);
    return 0;

}

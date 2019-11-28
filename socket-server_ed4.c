/*Simple udp server*/

/*
Edit version: 4 (4/4/2017)
Remarks     : stream dummy value of application rates in few seconds 
*/


#include<stdio.h> 		//printf
#include<string.h> 		//memset
#include<stdlib.h> 		//exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>		// close()
#include<time.h>
#include<math.h>

#define BUFLEN 512  //Max length of buffer
#define PORT 4445   //The port on which to listen for incoming data
 
void die(char *s)
{
    perror(s);
    exit(1);
}
     
int main(void)
{
    
    /*PARAMETERS*/
    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
    char str[10];
    float a; //dummy data
    clock_t start,end,timestamp;
    int time_len = 15; //seconds
    int min =100;
    int max =200;  

    /*PROCEDURES*/

    //1-create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    memset((char *) &si_me, 0, sizeof(si_me)); 
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //2-bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
    
    printf("Waiting for request...\n");
    fflush(stdout);
    
    //3-receive reply to start send data (establish connection)   
    if ((recv_len = recvfrom(s, buf, BUFLEN, 0, \
	(struct sockaddr *) &si_other, &slen)) == -1)
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

    //4-generate data, send, and receive back (for troubleshooting)
    time(&start);
    do
    {
	a  = rand()/(float)RAND_MAX*(max-min)+min;
	timestamp = time(NULL);
	time(&end);

	sprintf(str,"%.1f",a);
	strcpy(buf,str);
	     
	if(sendto(s,buf,strlen(buf),0,\
	    (struct sockaddr*) &si_other,slen) == -1)
		 die("sendto()");
        if (recvfrom(s,buf,BUFLEN,0,\
	    (struct sockaddr*)&si_other,&slen) == -1)
		 die("recvfrom()");
	
	printf("%s::SENSED RATE = ",ctime(&timestamp));
	puts(buf);
	if (strcmp(buf,"off") == 0)
	    die("Terminated");
	memset(str,'\0',BUFLEN);
	memset(buf,'\0',BUFLEN);
    }
    while(difftime(end,start) !=time_len);
    printf("Total duration: %f seconds.\n",difftime(end,start));
	
    close(s);
    return 0;
}

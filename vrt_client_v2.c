/*
Last update	: 17Nov2017
Notes	    	: vrt_client_v2.c
		- packetized information to |Control|Status|Mode|Data|
		- Control - connection 'heartbeat'
				:1 (connected), 0 (disconnected)
		- Status - START/STOP/PAUSE/RESUME
		- Mode   - AUTO/MANUAL
		- Data 
Updates		- both 'status' and 'mode' are written as function
                - client shall receive continuous message to server using sendto() in the
                  following format: "start(1/0),pause(1/0),gps-x(float),gps-y(float),manual(1/0),app-rate(int)".
*/
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     /*Unix Standard Definitions*/
#include <termios.h>    /*Posix Terminal Control Definitions*/
#include <fcntl.h>      /*File Control Definitions*/
#include <errno.h>      /*Error Number Definitions*/
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <stdbool.h>

#define SERVER "127.0.0.1" //local server
#define BUFLEN 512  //Max length of buffer
#define PORT 4445   //The port on which to send data
//////////////////////////////////////////////////////////////////////////////
void die(char *s)
{
    perror(s);
    exit(1);

}
//////////////////////////////////////////////////////////////////////////////
int main(void)
{
    /*UDP Socket Variables*/
    struct sockaddr_in si_other;
    int udp_socket, slen=sizeof(si_other);
    int recv_len;
    char rbuf[BUFLEN],msg[BUFLEN];
               
    /*----------------------------------------------------------------------*/
    /*Establish Connection*/
    /*----------------------------------------------------------------------*/
    if( (udp_socket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1)
	die("Unable to create udp_socket\n");
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);    
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
	fprintf(stderr, "inet_aton() failed\n");
	exit(1);
    }
    /*----------------------------------------------------------------------*/
    /*Generate Data: Client Info*/
    /*----------------------------------------------------------------------*/  
    bool isExit;
    int stat; char status[8];
    int mode; char modes[8];
    memset(msg,'\0',BUFLEN);
    memset(rbuf,'\0',BUFLEN);

    printf ("State : Press 1-START, 2-EXIT: ");
    scanf("%d",&stat);
    printf("Mode   : Press 1-AUTO, 2-MANUAL: ");
    scanf("%d",&mode);

    switch(stat)
    {
	case 1: strcpy(status,"START"); isExit = false;
	break;

	case 2: strcpy(status,"STOP"); isExit = true;
	break;

	//case 3: strcpy(status,"PAUSE");
	//break:

	default:strcpy(status,"START"); isExit = false;
    }

    switch(mode)
    {
	case 1: strcpy(modes,"AUTO");
	break;

	case 2: strcpy(modes,"MANUAL");
	break;

	default: strcpy(modes,"AUTO");
    }

    sprintf(msg,"%s,%s,",status,modes);
    printf("Data Sent:  %s\n",msg);
    if (sendto(udp_socket,msg,strlen(msg),0,
	(struct sockaddr *)&si_other,slen)==-1)
	die("Unable to send msg\n");
    fflush(stdout);
    /*----------------------------------------------------------------------*/
    /*Receive data from server*/
    /*----------------------------------------------------------------------*/
    int c;
    START: while(!isExit)
    {
        recv_len = 0;
        printf("recv_len = %d\n", recv_len);
        puts(rbuf);
        
	if((recv_len=recvfrom(udp_socket,rbuf,BUFLEN,0,
	    (struct sockaddr *)&si_other,&slen)) == -1)
	    die("Unable to receive from server\n");
	if (recv_len>0)
	{
	    printf("token\n");
            rbuf[recv_len]=0;
	    printf("Data Received: ");
	    puts(rbuf);

	    if(*rbuf == '#')
	    {
		strcpy(msg,"ok");
		sleep(1);
		if(sendto(udp_socket,msg,BUFLEN,0,                
		    (struct sockaddr *)&si_other,slen) == -1)
		    die("Unable to send msg2\n");
		fflush(stdout);
		printf("SENDING...\n");
	    }

	}

	printf("Press 1 to EXIT, 2 to PAUSE, any integer to continue: ");
	scanf("%d",&c);
	//puts("\n");
	if (c==1)
	{
	    strcpy(msg,"*");
	    sendto(udp_socket,msg,strlen(msg),0,
	    (struct sockaddr*)&si_other,slen);
	    isExit = true;
	}
	
	if(c==2)
	{
	    strcpy(msg,"p");
	    sendto(udp_socket,msg,BUFLEN,0,
	    (struct sockaddr*)&si_other,slen);
	    isExit = true;
        }

	memset(&msg,0,sizeof(msg));
        memset(&rbuf,0,sizeof(rbuf));
        bzero(rbuf, BUFLEN);
        bzero(msg, BUFLEN);
    }//end while(!isExit)

    STOP: if (c == 1)
    {
	close(udp_socket);
	die("Connection Aborted");
    }
    if (c == 2)
    {
	int a;
	printf("System PAUSED. Press 1 to resume : ");
	scanf("%d",&a);
	if(a == 1)
	{
	    strcpy(msg,"r");
	    sendto(udp_socket,msg,BUFLEN,0,
	    (struct sockaddr*)&si_other,slen);
            memset(&msg,0,sizeof(msg));
            bzero(msg, BUFLEN);
	    isExit=false;
	    goto START;
	}
	else
	{
	    c = 1;
	    goto STOP;
	}
    }
    close(udp_socket);
    return 0;
}//end main
//////////////////////////////////////////////////////////////////////////////


/*
Last update	: 16Nov2017
Notes	    	: vrt_client.c
		- packetized information to |Control|Status|Mode|Data|
		- Control - connection 'heartbeat'
				:1 (connected), 0 (disconnected)
		- Status - START/STOP/PAUSE/RESUME
		- Mode   - AUTO/MANUAL
		- Data 
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

//#define SERVER "192.168.0.5"
//#define SERVER "10.47.118.156"
#define SERVER "127.0.0.1" //local server
//#define SERVER "10.47.118.79"
#define BUFLEN 512  //Max length of buffer
#define PORT 4445   //The port on which to send data
//////////////////////////////////////////////////////////////////////////////
void die(char *s)
{
    perror(s);
    exit(1);

}
//////////////////////////////////////////////////////////////////////////////
struct termios stdin_orig;
void term_reset() 
{
    tcsetattr(STDIN_FILENO,TCSANOW,&stdin_orig);
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&stdin_orig);
}
void term_nonblocking() 
{
    struct termios newt;
    tcgetattr(STDIN_FILENO, &stdin_orig);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // non-blocking
    newt = stdin_orig;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    atexit(term_reset);

}
/*CANNOT RETURN STRING
//////////////////////////////////////////////////////////////////////////////
int set_status(char stat)
{
    switch (stat)
    {
        case 1 :
        //return("START");
        return 1;
	break;
        
	case 2 :
        //return("STOP");
        return 2;
	break;

        case 3 :
        //return("PAUSE");
        return 3;
	break;

        case 4:
        //return("RESUME");
        return 4;
	break;

        default:
        //return("UnknownStatus");
	return 5;
	break;
    }
}
//////////////////////////////////////////////////////////////////////////////
int set_mode (char mode)
{
    switch (mode)
    {
        case 1 : 
        //return("AUTO");
	return 1;
        break;

        case 2 :
        //return("MANUAL");
	return 2;
        break;

        default: 
        //return("UnknownMode");
	return 3;
	break;
    }

}
*/
//////////////////////////////////////////////////////////////////////////////
int main(void)
{
    /*UDP Socket Variables*/
    struct sockaddr_in si_other;
    int udp_socket, slen=sizeof(si_other);
    int recv_len;
    char rbuf[BUFLEN];

    /*Packet Variables*/
    char pkt_msg[BUFLEN];
   

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
    /*Create Packet*/
    /*----------------------------------------------------------------------*/
    bool isExit;
    
    int stat;
    char status[8];
    printf ("State : Press 1-START,2-EXIT: ");
    scanf("%d",&stat);
    fflush(stdout);

    int mode;
    char modes[8];
    printf("Mode  : Press 1-AUTO, 2-MANUAL: ");
    scanf("%d",&mode);
    fflush(stdout);

    switch (stat)
    {
	case 1 :
	isExit = false; 
	strcpy(status,"START"); 
	break;
	case 2 :
	isExit = true;
	strcpy(status,"STOP");
	break;
	case 3 :
	strcpy(status,"PAUSE");
	break;
	case 4:
	strcpy(status,"RESUME");
	break;
	default:
	strcpy(status,"UnknownStatus");
    }

    switch (mode)
    {
	case 1 : 
	strcpy(modes,"AUTO");
	break;
	case 2 :
	strcpy(modes,"MANUAL");
	break;
	default: 
	strcpy(status,"UnknownMode");
    }

    sprintf(pkt_msg,"%s,%s",status,modes);
    printf("Data sent: %s\n",pkt_msg);
    if (sendto(udp_socket,pkt_msg,strlen(pkt_msg),0,
	(struct sockaddr *)&si_other,slen)==-1)
	die("Unable to send pkt_msg\n");
   /*----------------------------------------------------------------------*/

    term_nonblocking();
    if(!(status == "STOP" || status == "UnknownStatus"))
	isExit = false;
    char c;

    while(!isExit)
    {
	recv_len =recvfrom(udp_socket,rbuf,BUFLEN,0,
	    (struct sockaddr *)&si_other,&slen);
	
	if (recv_len > 0)
	{
	    rbuf[recv_len]=0;
	    if(*rbuf == '*')
	    {	
		printf("In progress: %s\n",rbuf);
		isExit = true;
	    }

	    if(*rbuf == '#')
	    {
		printf("Data received: ");
		puts(rbuf);
		strcpy(pkt_msg,"ok");
		if(sendto(udp_socket,pkt_msg,strlen(pkt_msg),0,
		    (struct sockaddr *)&si_other,slen)== -1)
		    die("Unable to send to server");
	    }
	}
	else
	    die("Unable to receive from server\n");

	if((c = getchar())=='2'|| (c = getchar())=='3')
	    isExit = true;
//	if((c=getchar())=='3')
//	    isExit = true;

	fflush(stdout);
    }//end while
    if (c == '2')//stop
    {
	strcpy(pkt_msg,"*");
	if(sendto(udp_socket,pkt_msg,strlen(pkt_msg),0,
	    (struct sockaddr*)&si_other,slen) == -1)
	    die("Unable to sendto()");
    }
    if (c == '3')//pause
    {
	strcpy(pkt_msg,"p");
	if(sendto(udp_socket,pkt_msg,strlen(pkt_msg),0,
	    (struct sockaddr*)&si_other,slen) == -1)
	    die("Unable to sendto()");
	if (getchar()=='4')
	{
	    strcpy(pkt_msg,"r");
	    if(sendto(udp_socket,pkt_msg,strlen(pkt_msg),0,
		(struct sockaddr*)&si_other,slen) == -1)
		die("Unable to sendto() at PAUSE");
    	}
   } 


    die("Connection Aborted");
    close(udp_socket);
    return 0;
}//end main
//////////////////////////////////////////////////////////////////////////////


/*
Last Update	: 17Nov2017
Notes   	: vrt_server_manual.c (No sensor - dummy data)
        - has dual mode (auto/manual)
                - auto  - server send CropCircle data to client
                - manual- client send data to set the calibrator 
        - has interrupt handler (PAUSE/STOP/RESUME)
                - PAUSE  - put server send() in halt
                - STOP   - exit system
                - RESUME - continue 
        - server shall receive continuous message from client using recvfrom() in the
          following format: "start(1/0),pause(1/0),gps-x(float),gps-y(float),manual(1/0)".
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
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>

#define PORT_SENSOR "/dev/ttyS5"
#define PORT_CALIBRATOR "/dev/ttyS7"
#define PORT_UDP 4445
#define BUFLEN 512
#define Ntime 1


//////////////////////////////////////////////////////////////////////////////
int die (char *s)
{
    perror(s);
    exit(-1);
}
//////////////////////////////////////////////////////////////////////////////
int port_sensor(void)
{
    int port;
    port = open(PORT_SENSOR,O_RDWR | O_NOCTTY | O_NDELAY);
    if (port <1)
	die("Unable to open port_sensor\n ");
    else
	fcntl(port,F_SETFL,0);
    return (port);
}
int sensorPortsettings(int fsensor)
{
    struct termios specs;
    /*Modify port settings*/
    cfsetospeed(&specs,B38400);
    cfsetispeed(&specs,B38400);
    specs.c_cflag |= (CS8 | CREAD | CLOCAL |CRTSCTS);
    specs.c_cflag &= ~(PARENB | CSTOPB );
    specs.c_lflag |= ICANON;
    specs.c_iflag |=  (IGNPAR | ICRNL);
    specs.c_oflag  = 0;
    specs.c_cc[VMIN] = 1;
    specs.c_cc[VTIME]= 0;
    /*Clear port line and set modified port settings*/
    tcflush(fsensor, TCIOFLUSH);
    if (tcsetattr(fsensor,TCSANOW,&specs) != 0) 
	die("tcsetattr:Unable to set sensorPortsettings");
}
//////////////////////////////////////////////////////////////////////////////
int port_calibrator(void)
{
    int port;
    port = open(PORT_CALIBRATOR,O_RDWR | O_NOCTTY | O_NDELAY);
    if (port <1)
	die("Unable to open port_calibrator port\n");
    else
	fcntl(port,F_SETFL,0);
    return (port);
}
int calibratorPortsettings (int fcal)
{
    struct termios specs;  
    /*Modify port settings*/
    cfsetospeed(&specs,B9600);
    cfsetispeed(&specs,B9600);
    specs.c_cflag |= (CS8 | CREAD | CLOCAL);
    specs.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);
    specs.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    specs.c_oflag &= ~OPOST;
    specs.c_cc[VMIN] = 1;
    specs.c_cc[VTIME]= 0;
    //--Clear port line and set modified port settings
    tcflush(fcal, TCIFLUSH);
    if (tcsetattr(fcal,TCSANOW,&specs) != 0)
	die("tcsetattr:Unable to set calibratorPortsettings");
}
//////////////////////////////////////////////////////////////////////////////
int csum(char *buff)
{
    int len=strlen(buff);
    int i;
    if(buff[len-1]=='\n')
	buff[--len] = '\0';
    int csum=0;
    for(i =0; i<len; i++){
	csum=csum^buff[i];}
    return csum;
}
//////////////////////////////////////////////////////////////////////////////

int main(void)
{
    /*UDP Socket Variables*/
    struct sockaddr_in si_me, si_other;
    int udp_socket, slen=sizeof(si_other);
    int recv_len, n, start, pause, manual;
    float gps-x, gps-y;
    char rbuf[BUFLEN], msg[BUFLEN];
    struct timeval tv;
    
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    fd_set rset;

    /*----------------------------------------------------------------------*/
    /*Establish Connection*/
    /*----------------------------------------------------------------------*/
    LOOP:
    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        die("Unable to create udp_socket\n");
    memset((char *)&si_me,0,sizeof(si_me));
    memset((char *)&si_other,0,sizeof(si_other));
    si_me.sin_family      = AF_INET;
    si_me.sin_port        = htons(PORT_UDP);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if ((bind(udp_socket,(struct sockaddr*)&si_me,sizeof(si_me))) == -1)
        die("Unable to bind udp_socket\n");
    printf("Waiting for request..\n");
    /*----------------------------------------------------------------------*/
    /*Extract Client Info*/
    /*----------------------------------------------------------------------*/
    bool isExit;
    int stat; char status[8];
    int mode; char modes[8];
    //memset(msg,'\0',BUFLEN);
    //memset(rbuf,'\0',BUFLEN);

   //keep listening for data
    while(1)
    {
        
        fflush(stdout);
        FD_ZERO(&rset); 
	FD_SET(udp_socket, &rset);  
	
       
	n = select(udp_socket+1, &rset, 0, 0, &tv); 	
        //  n = select(s+1, &rset, 0, 0, 0); 	
 
	if(n < 0)
	  {
		perror("ERROR Server : select()\n");
		close(udp_socket);
		exit(1);
	  }

	if (FD_ISSET(udp_socket, &rset)) 
	{
	
		printf("Server is ready to read\n");

	        //try to receive some data, this is a blocking call
	        if ((recv_len = recvfrom(udp_socket, rbuf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
	        {
	            die("recvfrom()");
	        }
         
	        //print details of the client/peer and the data received
	        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
	        printf("Data: %s\n" ,rbuf);

                //masukkan code utk start, pause, manual, gps di sini.
                
                if(rbuf("start") == 1) //buat parsing ascii data macam baca dari sensor 
		{
		   start = 1;
                }
	        else
                {
		   start = 0;
                }
                
                if(rbuf("pause") == 1) //buat parsing ascii data macam baca dari sensor 
                {
		   pause = 1;
                }
	        else
                {
		   pause = 0;
                }
               
                if(rbuf(manual) == 1) //buat parsing ascii data macam baca dari sensor 
		{
		   manual = 1;
                }
	        else
                {
		   manual = 0;
                }   
                
                gps-x = rbuf(gps-x); // buat parsing ascii data macam baca dari sensor 
                gps-y = rbuf(gps-y); // buat parsing ascii data macam baca dari sensor 
		
		FD_CLR(s, &rset);
	 }
	 else 
	 {
         //masukkan code utk hantar sensor data ke tab di sini.
                
                if (sendto(udp_socket, msg, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
	        {
	            die("sendto()");
	        }
                
		printf("server ain't ready\n");
		sleep(1);
		
	 }
 
         memset(&rbuf, 0, sizeof(rbuf));
         bzero(rbuf, BUFLEN);
         memset(&msg, 0, sizeof(msg));
         bzero(msg, BUFLEN);
      
    }
    close(s);







     
    if((recv_len=recvfrom(udp_socket,rbuf,BUFLEN /*sizeof(rbuf)*/,0,
    //if((recv_len=recvfrom(udp_socket,rbuf,strlen(rbuf),0,
	(struct sockaddr*)&si_other,&slen)) == -1)
	die("Unable to receive from udp_socket\n");
    rbuf[recv_len]=0;
 
    strcpy(status,strtok(rbuf,","));
    strcpy(modes,strtok(NULL,","));
    printf("Source : %s\n", inet_ntoa(si_other.sin_addr));
    printf("Port   : %d\n", ntohs(si_other.sin_port));
    printf("State  : %s\n", status);
    printf("Mode   : %s\n", modes);

    if(strcmp(status,"START")==0)
    {
	isExit = false;
	stat = 1;
    }
    if (strcmp(status,"STOP")==0)
    {
	isExit = true;
	stat = 2;
    }

    if(strcmp(modes,"AUTO")==0 && stat !=2)
	mode = 1;
    if(strcmp(modes,"MANUAL")==0 && stat !=2)
	mode = 2;
     
    memset(&msg,0,sizeof(msg));
    memset(&rbuf,0,sizeof(rbuf));
    bzero(rbuf, BUFLEN);
    bzero(msg, BUFLEN);
    //memset(rbuf,'\0',BUFLEN);
    //recv_len=0;
    /*----------------------------------------------------------------------*/
    /*START*/
    /*----------------------------------------------------------------------*/
    START: if (stat == 1)
    {
	switch(mode)
	{
	    case 1: break;
            /*--------------------------------------------------------------*/
	    /* Manual mode*/
	    /*--------------------------------------------------------------*/
	    case 2: 
            printf("token4\n");
	    while(!isExit)
	    {
		/*----------------------------------------------------------*/
		/*Generate data: dummy*/
		/*----------------------------------------------------------*/
		//strcpy(msg,"#,0.1,0.2,0.3,0.4,0.5,0.6,0.7");
                strcpy(msg,"#,1");
                printf("token2\n");
		/*----------------------------------------------------------*/
		/*Send data to client*/
		/*----------------------------------------------------------*/
                sleep(1);
		if(sendto(udp_socket,msg,strlen(msg),0,(
		    struct sockaddr*)&si_other,slen) == -1)
		    die("Unable to sendto()");
	
		/*----------------------------------------------------------*/
		/*Receive data from client*/
		/*----------------------------------------------------------*/
		if((recv_len=recvfrom(udp_socket,rbuf,BUFLEN /*sizeof(rbuf)*/,0,
		    (struct sockaddr*)&si_other,&slen)) == -1)
		    die("Unable to recvfrom");

		if(recv_len > 0)
		{
		    //rbuf[recv_len]=0;
		    printf("strlen:%d, Data Received ",recv_len);
		    puts(rbuf);

		    if(*rbuf == '*')
		    {
			stat = 2;
			isExit=true;
			printf("STOP: isExit(true)\n");
		    }

		    if(*rbuf == 'p')
		    {
			stat = 3;
			isExit=true;
			printf("PAUSE: isExit(true)\n");
		    }
		    fflush(stdout);
		}//end recv_len > 0
		//else
	    memset(&msg,0,sizeof(msg));
            memset(&rbuf,0,sizeof(rbuf));
            bzero(rbuf, BUFLEN);
            bzero(msg, BUFLEN);
            fflush(stdout);
	    }//end while(!isExit)
	    //memset(msg,'\0',BUFLEN);
	    //memset(rbuf,'\0',BUFLEN);
	    break;
	}//end swicth(mode)
    }//end stat=1


    /*----------------------------------------------------------------------*/
    /*STOP*/
    /*----------------------------------------------------------------------*/
    STOP: if(stat ==2)
    {
	close(udp_socket);
	goto LOOP;
    }
    /*----------------------------------------------------------------------*/
    /*PAUSE*/
    /*----------------------------------------------------------------------*/
    PAUSE: if(stat ==3)
    {
        printf("token\n");
	if(recvfrom(udp_socket,rbuf,BUFLEN,0,
	    (struct sockaddr*)&si_other,&slen)==-1)
	    die("Unable to recvfrom() at PAUSE");
	if (*rbuf == '*')
	{
	    stat=2;
	    goto STOP;
	}
	if (*rbuf == 'r')
	{
            printf("token3\n");
	    stat = 1;
            memset(&msg,0,sizeof(msg));
            memset(&rbuf,0,sizeof(rbuf));
            bzero(rbuf, BUFLEN);
            bzero(msg, BUFLEN);
            fflush(stdout);
            strcpy(msg,"wassap");
            close(udp_socket);
	    goto START;
      
	}

    }     
       
    /*----------------------------------------------------------------------*/
    /*EXIT*/
    /*----------------------------------------------------------------------*/
    close(udp_socket);
    return 0;
}
//////////////////////////////////////////////////////////////////////////////

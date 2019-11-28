/*
    Simple udp client
*/

/*
Edit version: 4 (4/4/2017)
Remarks     : Display application rate for 10 minutes. Enter 2 to force exit.
*/

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdbool.h>
#include<termios.h> 
#include<unistd.h>
#include<fcntl.h>

#define SERVER "127.0.0.1"
//#define SERVER "10.47.118.52"
//#define SERVER "10.47.118.81"
#define BUFLEN 512  //Max length of buffer
#define PORT 4445   //The port on which to send data

void die(char *s)
{
    perror(s);
    exit(1);
}

void isExit(bool a)
{
   if (a == true)
     die("SELECT: EXIT(0)");
}

void isStart(bool a)
{
   if (a == true)
     printf("SELECT: START(1)\n");
}


struct termios stdin_orig;  // Structure to save parameters

void term_reset() {
        tcsetattr(STDIN_FILENO,TCSANOW,&stdin_orig);
        tcsetattr(STDIN_FILENO,TCSAFLUSH,&stdin_orig);
}

void term_nonblocking() {
        struct termios newt;
        tcgetattr(STDIN_FILENO, &stdin_orig);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // non-blocking
        newt = stdin_orig;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        atexit(term_reset);
}
 
int main(void)
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    int c;
    c = 0;    

    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    } 

   //start?
   int start;
   printf("Press 1 to start, or 2 to exit: ");
   scanf("%d",&start);
   fflush(stdout);

   if (start != 1)
	isExit(true);
   else 
	isStart(true);

   for(i = 0; i<45; i++)
	printf("*");
   printf("\n");

    //waiting for server
    strcpy(message,"on");
    if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
	{
            die("sendto()");
        }
      
   term_nonblocking(); 
   //loop for receive and send
   int count = 0;
   int opt;
   //for(;;)
     do    
     {
    memset(buf,'\0', BUFLEN);
	//received from server
    if(recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
	{
		die("recvfrom()");
        }
    printf("Data received: ");    
    puts(buf); sleep(1);    
  	//sent back to server
    if (sendto(s, buf, BUFLEN, 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
	
    
    
    }while ((c= getchar()) !='2' ); //end of while
      
	  strcpy(message,"off");
	  sendto(s, message, BUFLEN , 0 , (struct sockaddr *) &si_other, slen);
	  isExit(true);
	
    close(s);
    return 0;
}

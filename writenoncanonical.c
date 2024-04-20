/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
   int fd,c, res;
   struct termios oldtio,newtio;
   unsigned char bufw[5];
   unsigned char bufr[5];
   int i, sum = 0, speed = 0, STATE=0;

   if ( (argc < 2) ||
        ((strcmp("/dev/ttyS0", argv[1])!=0) &&
         (strcmp("/dev/ttyS1", argv[1])!=0) )) {
       printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
       exit(1);
   }


   /*
   Open serial port device for reading and writing and not as controlling tty
   because we don't want to get killed if linenoise sends CTRL-C.
   */


   fd = open(argv[1], O_RDWR | O_NOCTTY );
   if (fd < 0) { perror(argv[1]); exit(-1); }

   if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
       perror("tcgetattr");
       exit(-1);
   }

   bzero(&newtio, sizeof(newtio));
   newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
   newtio.c_iflag = IGNPAR;
   newtio.c_oflag = 0;

   /* set input mode (non-canonical, no echo,...) */
   newtio.c_lflag = 0;

   newtio.c_cc[VTIME]    = 50;   /* inter-character timer unused */
   newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



   /*
   VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
   leitura do(s) próximo(s) caracter(es)
   */


   tcflush(fd, TCIOFLUSH);

   if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
       perror("tcsetattr");
       exit(-1);
   }

   printf("New termios structure set\n");


/*
   for (i = 0; i < 255; i++) {
       buf[i] = 'a';
   }
*/

   bufw[0] = 0x5c;
   bufw[1] = 0x5C;//01
   bufw[2] = 0x5C;//08
   bufw[3] = bufw[1]^bufw[2];
   bufw[4] = 0x5c;       

   /*testing*/
   res = write(fd,bufw,5);
   printf("%d bytes written\n", res);

   for (i = 0; i < 5; i++) {
       printf("%02x \n", (unsigned char)bufw[i]);
   }

   printf("Receive: \n");
   
   if(read(fd,bufr,5) < 0){
      perror("Erro de leitura");
   }
   else{
      for(i = 0; i < 5; i++){
         printf("%02x\n", bufr[i]);
      }
   }

    unsigned char XOR=0x03^0x06;
      i = 0;
  while(STOP == FALSE && i <= 4){
    if(STATE == 0){
        printf("STATE: %d\n", STATE);
         STATE++;
         
    }

    switch (STATE)
    {
    case 1:
        printf("STATE: %d\n", STATE);
        while(bufr[i] == 0x5c) i++;
        
        if(bufr[i] == 0x03) STATE++;

        else STATE = 0;
        break;
     
    case 2:
        printf("STATE: %d\n", STATE);
        if(bufr[i] == 0x06) STATE = 3;

        else if(bufr[i] == 0x5c){
         STATE = 1;
         }
        else STATE = 0;
        
        break;
     
    case 3:
        printf("STATE: %d\n", STATE);
        if(bufr[i] == XOR){
            STATE = 4;
        }
        else if(bufr[i] == 0x5c){
         STATE = 1;
         }
        else STATE = 0;
        break;
     
    case 4:
        printf("STATE: %d\n", STATE);
        if(bufr[i] == 0x5c){ 
            STATE = 5;
            STOP = TRUE;
        }
        else if(bufr[i] == 0x5c){
            STATE = 1;
        }
        else STATE = 0;

        break;
    
    default:        
        break;
    }
    i++;
}
    if (STOP == TRUE){
        printf("UA bem recebido!\n");
    }
    else printf("UA mal!\n");

   /*
   O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar
   o indicado no guião
   */
   
   if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
       perror("tcsetattr");
       exit(-1);
   }

   close(fd);
   return 0;
}
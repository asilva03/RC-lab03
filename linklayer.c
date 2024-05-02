#ifndef LINKLAYER
#define LINKLAYER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct linkLayer{
    char serialPort[50];
    int role; //defines the role of the program: 0==Transmitter, 1=Receiver
    int baudRate;
    int numTries;
    int timeOut;
} linkLayer;

//ROLE
#define NOT_DEFINED -1
#define TRANSMITTER 0
#define RECEIVER 1


//SIZE of maximum acceptable payload; maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

//CONNECTION deafault values
#define BAUDRATE_DEFAULT B38400
#define MAX_RETRANSMISSIONS_DEFAULT 3
#define TIMEOUT_DEFAULT 4
#define _POSIX_SOURCE 1 /* POSIX compliant source */

//MISC
#define FALSE 0
#define TRUE 1

int fd,c, res;
  
   struct termios oldtio,newtio;
   unsigned char bufw[255], bufr[255];
   unsigned char buf;
   int i=0, sum = 0, speed = 0, STATE=0;

// Opens a connection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters);
// Sends data in buf with size bufSize
int llwrite(unsigned char* buf, int bufSize);
// Receive data in packet
int llread(unsigned char* packet);
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(linkLayer connectionParameters, int showStatistics);



int llopen(linkLayer connectionParameters){
 fd = open(argv[1], O_RDWR | O_NOCTTY ); 
   if (fd < 0) { perror(argv[1]); exit(-1); }

   //guarda os parametros associados a fd em oldtio
   if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
       perror("tcgetattr");
       exit(-1);
   }
  
   
   bzero(&newtio, sizeof(newtio));
   newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;)
   newtio.c_iflag = IGNPAR;
   newtio.c_oflag = 0; 
   newtio.c_lflag = 0;
    /*
    A configuração newtio.c_lflag = 0; desativa todos os modos de operação local do terminal.
    isso inclui modos como ICANON (modo canônico) e ECHO (eco dos caracteres digitados). 
    Quando esses modos estão desativados, o terminal opera em modo não canônico, o que significa que a entrada é processada
    imediatamente, caractere por caractere, sem esperar por uma nova linha ou qualquer outra interação do usuário. 
    Além disso, nenhum eco dos caracteres digitados é feito. 
    Isso é útil em situações em que se deseja um controle total sobre o processamento da entrada e saída do terminal.
    */
   newtio.c_cc[VTIME]    = 50;   /* inter-character timer unused. Da 1s de timeout */ 
   newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */
   
/*    newtio.c_lflag = 0;:
       c_lflag é um membro da estrutura termios que controla os modos de operação local do terminal.
       A configuração newtio.c_lflag = 0; desativa todos os modos de operação local do terminal. 
       Isso inclui modos como ICANON (modo canônico) e ECHO (eco dos caracteres digitados).

   newtio.c_cc[VTIME] = 0;:
       c_cc é um array que armazena os caracteres especiais do terminal, como controle de fluxo, caractere de final de linha, entre outros.
       VTIME é uma constante que representa o índice do caractere de temporização de leitura.
   newtio.c_cc[VTIME] = 0; define o temporizador de caractere como zero, 
       o que significa que não há temporização entre caracteres. Isso indica que a função read()
       retornará imediatamente após ler o número mínimo de caracteres especificado pela configuração VMIN.

   newtio.c_cc[VMIN] = 5;:
       VMIN é uma constante que representa o índice do número mínimo de caracteres 
       para satisfazer uma chamada de leitura.
   ewtio.c_cc[VMIN] = 5; configura o número mínimo de caracteres para 5. 
       Isso significa que a função read() bloqueará até que pelo menos 5 caracteres sejam recebidos antes de retornar.
*/


   /*
   VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
   leitura do(s) próximo(s) caracter(es)
   */

   //limpa o buffer após escrever tudo
   tcflush(fd, TCIOFLUSH);

   //define os parametros associados
   if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
       perror("tcsetattr");
       exit(-1);
   }
   /*Apartir deste momento, o terminal está corretamente configurado para manipulação dos dados
    com as configurações da estrutura newtio*/
   printf("New termios structure set\n");

 
   bufw[0] = 0x5c;
   bufw[1] = 0x03;
   bufw[2] = 0x06;
   bufw[3] = bufw[1] ^ bufw[2];
   bufw[4] = 0x5c;

   for (i=0;i<5;i++){
       printf("%02x \n", bufw[i]);
   }
   
   printf("Receive:\n");

   if((res = read(fd,bufr,1)) < 0) perror("Erro de leitura");
   
   for(i = 0; i < 5; i++) printf("%02x\n", bufr[i]);
   

   unsigned char XOR = 0x01 ^ 0x08;
    i = 0;
  while(STOP == FALSE && i <= 4){
  res = read(fd,bufr,1);
  
    if(STATE == 0){ 
        printf("STATE: %d\n", STATE);
        STATE++;
        
    }

    switch (STATE)
    {
    case 1:
        printf("STATE: %d\n", STATE);
        while(bufr[i] == 0x5c) i++;
        
        if(bufr[i] == 0x01) STATE++;
        else STATE = 0;
        break;
     
    case 2:
        printf("STATE: %d\n", STATE);
        if(bufr[i] == 0x08) STATE = 3;

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
            STATE = 6;
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
        res = write(fd, bufw, 5);
    }
    else {
        perror("UA não enviado");
    }

   /*o terminal volta as configurações originais devido ao facto de certas aplicações 
     não definem as proprias configurações de terminal, por isso é importante voltar as definições originais*/
 

}
 llwrite(unsigned char* buf, int bufSize);
 
 int llread(unsigned char* packet){
 unsigned char frase[20];
    
    while(STOP == FALSE){
    if(STATE == 0){ 
        printf("STATE: %d\n", STATE);
        STATE++;
        
    }

    switch (STATE)
    {
    case 1:
        printf("STATE: %d\n", STATE);
        while(packet[i] == 0x5c) i++;
        
        if(packet[i] == 0x01) STATE++;
        else STATE = 0;
        break;
     
    case 2:
        printf("STATE: %d\n", STATE);
        if(packet[i] == 0x08) STATE = 3;

        else if(packet[i] == 0x5c){
         STATE = 1;
         }
        else STATE = 0;
        break;
     
    case 3:
        printf("STATE: %d\n", STATE);
        if(packet[i] == XOR){
            STATE = 4;
        }
        else if(packet[i] == 0x5c){
         STATE = 1;
         }
        else STATE = 0;
        break;
    
     
    case 4:
        printf("STATE: %d\n", STATE);
        while(packet[i] != 0x5c){
        packet[i] = frase[j];
            j++; 
            i++;
        }
        break;
    
    default:   
        break;
    }
    i++;
}

#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>//Fornece o uso das funções read() e write()
#include <fcntl.h>/* Fornece várias funções relacionadas ao controle de descritores de arquivos.
open(), close(), fcntl e fcntl(fd, F_GETFL) e fcntl(fd, F_SETFL, flags) estão todas relacionadas
ao controle de descritores de arquivo, mas cada uma executa uma tarefa diferente. Trabam com flags 
de status do descritor de arquivo(variável inteira associada ao arquivo).*/
#include "linklayer.h"
//noncanonical.c escreve na porta 0
/*Portanto, o programa noncanonical.c lida com a entrada e saída de dados da porta serial de forma "não convencional"
 ou "não canônica", ou seja, sem a interpretação especial de caracteres de controle. Isso pode ser útil em situações
  em que você precisa lidar com dados binários ou em que deseja ter mais controle sobre como os dados são tratados,
sem depender de convenções específicas de formatação de texto.*/

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define bufSize 30


int main(int argc, char** argv)
{
    
   linkLayer configsp;
   unsigned char *bufr, *bufw;
   unsigned char packet[bufSize];
   unsigned char dados[MAX_PAYLOAD_SIZE];
   
   /* recebe a segunda string dada pelo terminal(strings escritas por nós pelo terminal 
   que são separadas por ' ').
   */
   if ( (argc < 2) ||
        ((strcmp("/dev/ttyS0", argv[1])!=0) &&
         (strcmp("/dev/ttyS1", argv[1])!=0) )) {
       printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
       exit(1);
   }
   strcpy(configsp.serialPort, argv[1]);
   configsp.role = 1;
   configsp.baudRate = BAUDRATE;

   if(llopen(configsp) == -1) {
        perror("Erro na configuração\n");
        exit(-1);
   }
   if(llread(packet) < 0) {
    perror("Erro de leitura dos dados\n");
    exit(-1);
   if( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
       perror("tcsetattr");
       exit(-1);
   }

   close(fd);
   return 0;
}
}

/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h> // termios.h é uma biblioteca que fornece a estrutura necessária guardar 
#include <stdio.h>  //todos os parâmentros das portas portas série (oldtio e newtio).
#include <stdlib.h> //Também fornece as funções tcgetattr(), tcflush(), tcsetattr().
#include <string.h>
#include <unistd.h>//Fornece o uso das funções read() e write()
#include <fcntl.h>/* Fornece várias funções relacionadas ao controle de descritores de arquivos.
open(), close(), fcntl e fcntl(fd, F_GETFL) e fcntl(fd, F_SETFL, flags) estão todas relacionadas
ao controle de descritores de arquivo, mas cada uma executa uma tarefa diferente. Trabam com flags 
de status do descritor de arquivo(variável inteira associada ao arquivo).
*/

#define BAUDRATE B38400 // Número de simbolos(bits) transmitidos por segundo, ou seja 38400 b/s.
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
   int fd,c, res;
   struct termios oldtio,newtio;
   unsigned char buf[5];
   int i, sum = 0, speed = 0;
   //temos que criar uma maquina de estados de leitura
   //mudamos que estado sempre que recebemos uma flag diferente


   /* recebe a segunda string dada pelo terminal(strings escritas por nós pelo terminal 
   que são separadas por ' ').
   */
   if ( (argc < 2) ||
        ((strcmp("/dev/ttyS0", argv[1])!=0) &&
         (strcmp("/dev/ttyS1", argv[1])!=0) )) {
       printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
       exit(1);
   }
   /*
   /dev/ttyS0 é um caminho de dispositivo no sistema de arquivos do Linux que 
   representa uma porta serial de hardware.
   
   dev: Este é o diretório no sistema de arquivos do Linux onde estão 
   localizados os arquivos de dispositivos.
   
   ttyS0: Este é o nome do dispositivo serial. 
   No Linux, os dispositivos seriais são nomeados como ttyS0, ttyS1, ttyS2, e assim por diante. 
   O número após o ttyS indica a porta serial específica.
   Por exemplo, /dev/ttyS0 representa a primeira porta serial no sistema.

   Open serial port device for reading and writing and not as controlling tty
   because we don't want to get killed if linenoise sends CTRL-C.
   */
   
   //O_RDWR indica que o arquivo será aberto para leitura e escrita.
   //O_NOCTTY indica que o arquivo não será o terminal de controle do processo.

   //define fd como int associado a argv
   fd = open(argv[1], O_RDWR | O_NOCTTY ); //O |,  é uma fomra dos SO linux e Unix, de
   if (fd < 0) { perror(argv[1]); exit(-1); }//adicionar funções a função

   //guarda os parametros associados a fd em oldtio
   if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
       perror("tcgetattr");
       exit(-1);
   }
  
   //Inicializa o struct a zero e preenche 3 flags
   bzero(&newtio, sizeof(newtio));
   newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
   newtio.c_iflag = IGNPAR;
   newtio.c_oflag = 0;

   /* set input mode (non-canonical, no echo,...) */
   newtio.c_lflag = 0;

   newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
   newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */
   
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

   printf("New termios structure set\n");

 
   buf[0] = 0x5c;
   buf[1] = 0x03;
   buf[2] = 0x06;
   buf[3] = buf[1] ^ buf[2];
   buf[4] = 0x5c;



   //escreve no ficheiro associado a fd 255 caracteres do buf
   res = write(fd,buf,5);
  printf("%d bytes written\n", res);
   /*
   O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar
   o indicado no guião
   */
   for ( int i=0;i<5;i++){
       printf("%02x \n", buf[i]);
   }

   //define os parametros de oldtio
   if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
       perror("tcsetattr");
       exit(-1);
   }


   close(fd);
  return 0;
}   

/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400// Se aumentar a taxa de transmissão para 115200, as vezes não se recebe os frames
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
//writenoncanonical.c escreve na porta 1
volatile int STOP=FALSE;
//Usar o comando -> man "função a pesquisar" para saber mais sobre a mesma, no terminal do SO linux
//Atenção que devido a codificação UTF-8, os caracteres especiais podem ter tamanho de 2 bytes
int main(int argc, char** argv)
{
   int fd,c, res;
   struct termios oldtio,newtio;
   unsigned char bufr[255];
   unsigned char bufw[255];
   int i, sum = 0, speed = 0;

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

   if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings, Terminal Control get attributes = tcgetattr*/
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

   /*newtio.c_cc[VTIME] = 10 significa que ocorrerá um timeout de 1 segundo se não ocorrer 
   nehuma leitura por parte da função read. Se for igual a 0, fica a aguardar indefinidamente uma entrada.
   Se a conexão entre as portas estiver constantemente enviando dados, como parece ser o caso de sua situação
    em que uma porta está conectada à outra e há uma transmissão contínua de dados, o VTIME pode não ser eficaz
     para determinar quando sair da função read, porque a porta sempre estará "ativa", mesmo que não haja dados
      sendo enviados no momento exato da chamada da função.
      O VTIME é mais útil em situações em que você está esperando por entrada do usuário ou em comunicações em que
       há pausas entre a transmissão de dados.
   */

   /*
   VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
   leitura do(s) próximo(s) caracter(es)
   */

   tcflush(fd, TCIOFLUSH); //Terminal Control Input/Output FLUSH
   //Terminal Control Set Attributes = tsetattr
   if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
       perror("tcsetattr");
       exit(-1);
   }

   printf("New termios structure set\n");

   bufw[0] = 0x5c;
   bufw[1] = 0x01;
   bufw[2] = 0x08;
   bufw[3] = bufw[1] ^ bufw[2];
   bufw[4] = 0x5c;       

   /*testing*/
   res = write(fd,bufw,5);
   printf("%d bytes written\n", res);

   for (i = 0; i < 5; i++) {
       printf("%02x \n", (unsigned char)bufw[i]);
   }
   lseek(fd, 0, SEEK_SET);
   printf("Receive: \n");

   int k;

    if((k = read(fd,bufr,5)) < 0) perror("Erro de leitura");
    
    else for(i = 0; i < 5; i++) printf(": %02x\n", bufr[i]);
   
   /*for (int j = 0; j < 5; j++) {
       printf("%02x \n", (unsigned char)bufr[j]);
   }*/


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

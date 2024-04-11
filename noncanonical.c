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
   char buf[255];
   int i, sum = 0, speed = 0;
   /*
   oldtio: Esta é uma estrutura que armazena os atributos do
   terminal antes de serem modificados. É comum salvar esses atributos antes de fazer mudanças no 
   terminal para que possam ser restaurados posteriormente quando não forem mais necessários. 

   newtio: Esta é uma estrutura que armazena os novos atributos do terminal que serão aplicados.
   Quando você deseja modificar os atributos do terminal, você preenche a estrutura newtio com os valores
   desejados e aplica essas alterações ao terminal. 
   */


   // se não estiver a receber nada escreve
   if ( (argc < 2) ||
        ((strcmp("/dev/ttyS0", argv[1])!=0) &&
         (strcmp("/dev/ttyS1", argv[1])!=0) )) {
       printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
       exit(1);
   }
   /*/dev/ttyS0 é um caminho de dispositivo no sistema de arquivos do Linux que 
   representa uma porta serial de hardware.
   dev: Este é o diretório no sistema de arquivos do Linux onde estão 
   localizados os arquivos de dispositivos.

   ttyS0: Este é o nome do dispositivo serial. 
   No Linux, os dispositivos seriais são nomeados como ttyS0, ttyS1, ttyS2, e assim por diante. 
   O número após o ttyS indica a porta serial específica.
   Por exemplo, /dev/ttyS0 representa a primeira porta serial no sistema.*/


   /*
   Open serial port device for reading and writing and not as controlling tty
   because we don't want to get killed if linenoise sends CTRL-C.
   */
   //O_RDWR indica que o arquivo será aberto para leitura e escrita.
   //O_NOCTTY indica que o arquivo não será o terminal de controle do processo.

   //define fd como int associado a argv
   fd = open(argv[1], O_RDWR | O_NOCTTY );
   if (fd < 0) { perror(argv[1]); exit(-1); }

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



   for (i = 0; i < 255; i++) {
       buf[i] = 'a';
   }

   /*testing*/
   buf[25] = '\n';

   //escreve no ficheiro associado a fd 255 caracteres do buf
   res = write(fd,buf,255);
   printf("%d bytes written\n", res);


   /*
   O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar
   o indicado no guião
   */

   //define os parametros de oldtio
   if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
       perror("tcsetattr");
       exit(-1);
   }


   close(fd);
   return 0;
}

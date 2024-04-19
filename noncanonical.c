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
//noncanonical.c escreve na porta 0
/*Portanto, o programa noncanonical.c lida com a entrada e saída de dados da porta serial de forma "não convencional"
 ou "não canônica", ou seja, sem a interpretação especial de caracteres de controle. Isso pode ser útil em situações
  em que você precisa lidar com dados binários ou em que deseja ter mais controle sobre como os dados são tratados,
sem depender de convenções específicas de formatação de texto.*/

#define BAUDRATE B38400 // Número de simbolos(bits) transmitidos por segundo, ou seja 38400 b/s.
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
   int fd,c, res;
   /*file descriptor(fd).O parâmetro fd na função tcflush representa o 
   descritor de arquivo associado ao terminal que se deseja manipular.
   */
   struct termios oldtio,newtio;
   unsigned char bufw[255], bufr[255];
   int i=0, sum = 0, speed = 0, STATE=0;
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
   /*O_NOCTTY indica que o file descriptor (dev), associoado a porta serial ttyS0, por exemplo
    não será o terminal de controlo deste processo, ou seja, impede que ocorra interferências por parte
    do uso do terminal associado a este programa
    na ligação com outro file descriptor (dev), associado a uma porta serial ttyS1, por exemplo 
    */

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
   newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; //operador | (OR bit a bit)
   newtio.c_iflag = IGNPAR;
   newtio.c_oflag = 0; //desativa todos os flags de controle de saída da estrutura, útil quando não há necessidade de controle especial sobre a saída dos dados 
   /*
    BAUDRATE: Define a taxa de transmissão (baud rate) da comunicação serial.
    CS8: Configura o tamanho dos caracteres para 8 bits por byte.
    CLOCAL: Indica que a linha não é usada por um modem externo (ou seja, a conexão é local).
    CREAD: Ativa a recepção de caracteres.
    IGNPAR: Esta constante indica que os bytes de entrada com erros de paridade devem ser ignorado, sou seja, 
    o sistema operacional não reportará esses erros e os bytes serão tratados como se não contivessem erros.
   */

   /* set input mode (non-canonical, no echo,...) */
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

   if((res = read(fd,bufr,5)) < 0) perror("Erro de leitura");
   
   for(i = 0; i < 5; i++) printf("%02x\n", bufr[i]);
   

   unsigned char XOR = 0x01 ^ 0x08;
   
  i=0;
  while(STOP == FALSE && i <= 4){
    if(STATE == 0) {
        STATE++;
        i = 0;
    }

    switch (STATE)
    {
    case 1:
        i++;
        if(bufr[i] == 0x5c){
            STATE = 2;
        }
        else STATE = 1;
        break;
     
    case 2:
        i++;       
        if(bufr[i] == 0x01){
            STATE = 3;
        }
        else if(bufr[i] == 0x5c){
         STATE = 2;
         }
        
        else STATE = 1;
        break;
     
    case 3: 
        i++;
        if(bufr[i] == 0x08){
            STATE = 4;
        }
        else if(bufr[i] == 0x5c){
         STATE = 2;
         }
        else STATE = 1;
        break;
     
    case 4:
        i++;
        if(bufr[i] == XOR){; 
            STATE = 5;
        }
        else if(bufr[i] == 0x5c){
            STATE = 2;
        }
        //else printf("Erro ");
        else STATE = 1;
        break;
     
    case 5:
        if(bufr[i] == 0x5c){
            STATE = 6;
        }
        else STATE = 1;
        break;
    
    default:
        STOP = TRUE;        
        break;
    }
}
 
    if (STOP == TRUE){
        res = write(fd, bufw, 5);
    }
    else perror("UA não enviado");

   /*o terminal volta as configurações originais devido ao facto de certas aplicações 
     não definem as proprias configurações de terminal, por isso é importante voltar as definições originais*/
 
   if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
       perror("tcsetattr");
       exit(-1);
   }

   close(fd);
  return 0;
}   

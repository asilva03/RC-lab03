/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "linklayer.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define bufSize 30

int main(int argc, char **argv)
{
    linkLayer configsp;

    if ((argc < 2) ||
        ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
         (strcmp("/dev/ttyS1", argv[1]) != 0)))
    {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }
    strcpy(configsp.serialPort, argv[1]);
    configsp.role = 0;
    configsp.baudRate = BAUDRATE;

    if (llopen(configsp) == -1)
    {
        perror("Erro na configuração\n");
        exit(-1);
    }
    unsigned char dados[] = "Helicopter Helicopter"; // 22 caracteres, devido ao \0
    tam_dados = sizeof(dados);
    unsigned char bufw[bufSize];
    int n_packets = 0;

    if ((tam_dados / (bufSize - 6)) > 1)
        n_packets = (int)(tam_dados / (bufSize - 6)) + 1;
    else
        n_packets = 1;
    //Copia a informação de dados para bufw, até a posição do vetor de bufSize-1

    int n_byte_send = 0;

    while (n_packets)
    { /*Aqui assume-se que o cabeçalho é adicionado pela camada linklayer, logo os dados passados da aplicação para
a link layer não incluem o cabeçalho*/ 
        memcpy(bufw, &dados[sum], bufSize-1);

        if ((n_byte_send = llwrite(bufw, bufSize)) < 0)
        {
            perror("Error sending the information\n");
            exit(-1);
        }
        printf(" Pacote enviado com sucesso\n");
        printf("Número total de bytes enviados %d\n", n_byte_send);
        sum = sum + (n_byte_send - 6);
        // Para introduzir um TIMEOUT, não pode chamar a função read, se não fica bloqueado o programa
        // llread(fd, buffer, 1);
        // A chamada da função llread deverá ocorrer aqui
        n_packets--;
    }

    /*if(llread(&packet) < 0){
     perror("Error receiving the packet\n");
     exit(-1);
    }*/
    // llclose
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
    return 0;
}

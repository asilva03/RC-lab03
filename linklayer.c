

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "linklayer.h"


int llopen(linkLayer connectionParameters)
{
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    // guarda os parametros associados a fd em oldtio
    if (tcgetattr(fd, &oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }
    if (connectionParameters.baudRate != BAUDRATE_DEFAULT)
    {
        perror("BAUDRATE not fixed");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE_DEFAULT | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
   
    newtio.c_cc[VTIME] = 0; /*  inter-character timer unused. Da 1s de timeout */
    newtio.c_cc[VMIN] = 1;  /* blocking read until 5 chars received */
    
    // limpa o buffer após escrever tudo
    tcflush(fd, TCIOFLUSH);

    // define os parametros associados
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    /*Apartir deste momento, o terminal está corretamente configurado para manipulação dos dados
     com as configurações da estrutura newtio*/
    printf("New termios structure set\n");
    role = connectionParameters.role;
    unsigned char buffer;
    int STOP = FALSE, STATE = 0, i = 0;

    if (role == TRANSMITTER)
    { // Só para o noncanonical no final porque refere-se ao envio do AK
        i = 0;
        while (i <= 4)
        {

            if (i == 0)
                buffer = 0x5c;
            if (i == 1)
                buffer = 0x01;
            if (i == 2)
                buffer = 0x08;
            if (i == 3)
                buffer = 0x01 ^ 0x08;
            if (i == 4)
                buffer = 0x5c;
            printf("%02x\n", buffer);

            if (res = write(fd, &buffer, 1) < 0)
            {
                perror("Erro no envio do SET");
                return -1;
            }
            buffer++;

            i++;
        }
        printf("SET enviado com sucesso\n");
    }

    i = 0;
    unsigned char XOR;
    while (i <= 4)
    {

        res = read(fd, &buffer, 1); // Aqui, os programas encontra-se no estado 0 da máquina de estados a espera de receber alguma coisa do outro programa
        printf("%d \n", res);
        if (res < 0)
        {
            perror("Erro de leitura");
            return -1;
        }
        printf("%02x\n", buffer);

        if (STATE == 0)
        {
            printf("STATE: %d\n", STATE);
            STATE++;
        }

        switch (STATE)
        {
        case 1:
            printf("STATE: %d\n", STATE);
            if (buffer == 0x5c)
                break; // loop de flags 0x5c

            if (role == TRANSMITTER)
            {
                if (buffer == 0x03)
                    STATE++;
                else
                    STATE = 0;
            }

            if (role == RECEIVER)
            {
                if (buffer == 0x01)
                    STATE++;
                else
                    STATE = 0;
            }
            break;

        case 2:
            printf("STATE: %d\n", STATE);
            if (role == TRANSMITTER)
            {
                if (buffer == 0x06)
                {
                    STATE = 3;
                    break;
                }
            }

            if (role == RECEIVER)
            {
                if (buffer == 0x08)
                {
                    STATE = 3;
                    break;
                }
            }

            if (buffer == 0x5c)
                STATE = 1;
            else
                STATE = 0;
            break;

        case 3:
            printf("STATE: %d\n", STATE);
            if (role == TRANSMITTER)
            {
                XOR = 0x03 ^ 0x06;
                if (buffer == XOR)
                {
                    STATE = 4;
                    break;
                }
            }

            if (role == RECEIVER)
            {
                XOR = 0x01 ^ 0x08;
                if (buffer == XOR)
                {
                    STATE = 4;
                    break;
                }
            }

            if (buffer == 0x5c)
                STATE = 1;
            else
                STATE = 0;
            break;

        case 4:
            printf("STATE: %d\n", STATE);

            if (buffer == 0x5c)
                STOP = TRUE;
            else
                STATE = 0;
            break;

        default:
            break;
        }
        i++;
    }

    if (STOP == TRUE && role == RECEIVER)
    { // Só para o noncanonical no final porque refere-se ao envio do UA
        i = 0;
        while (i <= 4)
        {

            if (i == 0)
                buffer = 0x5c;
            if (i == 1)
                buffer = 0x03;
            if (i == 2)
                buffer = 0x06;
            if (i == 3)
                buffer = 0x03 ^ 0x06;
            if (i == 4)
                buffer = 0x5c;
            printf("%02x\n", buffer);

            if (res = write(fd, &buffer, 1) < 0)
            {
                perror("Erro no envio do UA");
                return -1;
            }
            i++;
        }
        printf("UA enviado com sucesso\n");
    }
    else if (role == RECEIVER)
    {
        printf("UA não enviado\n");
    }
    if (STOP == TRUE && role == TRANSMITTER)
        printf("UA recebido com sucesso\n");
    else if (role == TRANSMITTER)
        printf("UA não recebido\n");

    /*o terminal volta as configurações originais devido ao facto de certas aplicações
      não definem as proprias configurações de terminal, por isso é importante voltar as definições originais*/
    return 1;

} 
//TERMINA LLOPEN

/*while(sum <= bufSize){//sum serve para contar os bytes enviados para o recetor quando chegar ao MAX_PAYLOAD_SIZE, para
     unsigned char frase[] = "Helicopter Helicopter";/*temos que modular o tamanho de envio dos dados? Então eu tenho que criar um
     vetor auxiliar que envie os dados apartir do endereço recebido?*/
// bufSsize é o número de bytes total a ser enviados  e recebidos pela função llread*/

//COMEÇA LLWRITE
int llwrite(unsigned char *buf, int bufSize)
{
    int i = 0, j = 0; // i representa a posição do array aux_buf antes de calcular BCC2, j representa a posição de aux_buf para BCC2 e 0x5c
    unsigned char aux_buf[bufSize], BCC2;
    aux_buf[0] = 0x5c;
    aux_buf[1] = 0x01;
    aux_buf[2] = 0x08;
    aux_buf[3] = 0x01 ^ 0x08;
    printf("tam dados:%zu\n",tam_dados);

    while (i < (tam_dados + 6) && i < bufSize) // verificar a soma de 4
    {

        if (i <= 3)
        {
            if (write(fd, &aux_buf[i], 1) < 0)
            {
                perror("Erro no envio do packet");
                return -1;
            }
            printf("%02x \n", aux_buf[i]);
        }
        else if ((i < (bufSize - 2)) && (i < (tam_dados + 3)))
        // tam_dados + 3 porque: tam_dados + 4, trabalha-se com aux_buf[] depois do cabeçalho, -1 porque i representa a posição do array aux_buf[i]  
        // Esta ultima condição serve quando bufSize > (tam_dados + 6)
        {
            if(aux_buf[j] == 0x5c){
                    write(fd, "\x5d", 1);
                    write(fd, "\x7c", 1);
            }
            else if(aux_buf[j] == 0x5d){
                    write(fd, "\x5d", 1);
                    write(fd, "\x7d", 1);
            }
            else
            {
                if(write(fd, buf, 1) < 0){

                    perror("Erro de envio do packet\n");
                    return -1;
            }
            }
            
            aux_buf[i] = *buf;
            printf("%c", *buf);
            buf++;
            printf("Valor do i:%d\n", i);
        }
        else if (i <= (bufSize - 2) && (i <= (tam_dados + 3))) //28 != 25 e 
        {
            j = 4; // Posição do aux_buf onde os dados começam a ser guardados
            BCC2 = aux_buf[j];
            printf("Antes de enviar\n");
            printf("O valor atual de BCC2:%02x\n", BCC2);
            while (j < (bufSize - 2) && (j < (tam_dados + 2))){
                /* 
                Para tam_dados = 22, 22 + 4 -1 = 25,este valor corresponde á posição do array aux_buf que contém o ultimo byte de dados
                , o que equivale a tam_dados + 3 = 25, mas como se utiliza aux_buf[j + 1],fica tam_dados + 2 = 24, e como já se faz a
                primeira operação de XOR, é preciso decrementar ainda mais um, o que equivale a passar de j <= (tam_dados + 2)
                para j < (tam_dados + 2)
                */ 

                printf("%c\n",aux_buf[j + 1]);
                BCC2 = BCC2 ^ aux_buf[j + 1];
                printf("O valor atual de BCC2:%02x\n", BCC2);   // -3 porque, para um bufSize = 10 como exemplo, 10-3 = 7, 7+1 que corresponde
                j++;               

            }
            printf("Valor do BCC2:%02x\n", BCC2);
            aux_buf[j] = BCC2;
            printf("Enviei BCC2\n");
            if (write(fd, &aux_buf[j], 1) < 0)
            {
                perror("Erro de envio do packet\n");
                return -1;
            }
        }
        else
        {
            printf("Enviei a flag de terminação\n");
            printf("Valor do aux_buf antes de enviar a flag de terminação:%02x\n", aux_buf[j]);
            aux_buf[j + 1] = 0x5c;
            if (write(fd, &aux_buf[j + 1], 1) < 0)
            {
                perror("Erro de envio do packet\n");
                return -1;
            }
        }

        //STOP & WAIT PROTOCOL
        /*
        while(read(fd, wait, 1) != ACK){}
!!!!!!!!
        */
        i++;
    }
    return i;
}
//TERMINA LLWRITE

/*llread depois de feito o handshake, vai ficar a espera de receber os dados, o que quer dizer ao mesmo tempo que se está enviar um byte
, estará-se a receber o mesmo no outro lado, sistema FIFO. O envio pode ser mais rapida do que a receção, não origina erros?*/

//COMEÇA LLREAD
int llread(unsigned char *packet)
{
    //Usa-se alocação dinâmica porque não se sabe o tamanho do packet que vai chegar

    unsigned char *aux_packet = packet, BCC1, BCC2;
    unsigned char *bufr = (unsigned char *)malloc(sizeof(unsigned char));
    unsigned char *aux_bufr;
    int STOP = FALSE, STATE = 0, i = 0, cont = 0;
    while (STOP == FALSE)
    {

        if (STATE < 4) // Para não chamar a função read() ao ir para o estado da obtenção do XOR2
        {
            if (i != 0)
            {
                printf("Vou realocar memória, valor do i:%d\n", i);
                unsigned char *aux_bufr = realloc(bufr, (i + 1) * sizeof(unsigned char));
                if (aux_bufr == NULL)
                {

                    printf("Erro: Falha ao realocar memória.\n");
                    free(aux_bufr);
                    return -1;
                }
                else
                {

                    bufr = aux_bufr;
                }
            }
            res = read(fd, &bufr[i], 1);
            printf("Valor da primeira posição de bufr:%02x\n", bufr[i]);
            if (res < 0)
            {
                perror("Erro de leitura dos dados\n");
                return -1;
            }
        }

        if (STATE == 0)
        {
            printf("STATE: %d\n", STATE);
            STATE++;
        }

        switch (STATE)
        {
        case 1:
            printf("STATE: %d\n", STATE);

            printf("%02x\n", bufr[i]);
            if (bufr[i] == 0x5c)
            {
                i++;
                break; // loop de flags 0x5c
            }
            if (role == TRANSMITTER)
            {
                if (bufr[i] == 0x03)
                {
                    STATE++;
                    i++;
                }
                else
                {
                    STATE = 0;
                    i++;
                }
            }

            if (role == RECEIVER)
            {
                if (bufr[i] == 0x01)
                {
                    STATE++;
                    i++;
                }
                else
                {
                    STATE = 0;
                    i++;
                }
            }
            break;

        case 2:
            printf("STATE: %d\n", STATE);
            printf("%02x\n", bufr[i]);
            printf("%c\n", bufr[i]);
            if (role == TRANSMITTER)
            {
                if (bufr[i] == 0x06)
                {
                    STATE = 3;
                    i++;
                    break;
                }
            }

            if (role == RECEIVER)
            {
                if (bufr[i] == 0x08)
                {
                    STATE = 3;
                    i++;
                    break;
                }
            }

            if (bufr[i] == 0x5c)
            {
                STATE = 1;
                i++;
            }
            else
            {
                STATE = 0;
                i++;
            }
            break;

        case 3:
            printf("STATE: %d\n", STATE);
            printf("%02x\n", bufr[i]);
            printf("%c\n", bufr[i]);
            if (role == TRANSMITTER)
            {
                BCC1 = 0x03 ^ 0x06;
                if (bufr[i] == BCC1)
                {
                    STATE = 4;
                    i++;
                    break;
                }
            }

            if (role == RECEIVER)
            {
                BCC1 = 0x01 ^ 0x08;
                if (bufr[i] == BCC1)
                {
                    STATE = 4;
                    i++;
                    break;
                }
            }

            if (bufr[i] == 0x5c)
            {
                STATE = 1;
                i++;
            }
            else
            {
                STATE = 0;
                i++;
            }
            break;

        case 4:
            printf("STATE: %d\n", STATE);
            // printf("%c buffer\n",bufr);
            while (1)
            {
                
                unsigned char *aux_bufr = realloc(bufr, (i + 1) * sizeof(unsigned char));
                if (aux_bufr == NULL)
                {

                    printf("Erro: Falha ao realocar memória.\n");
                    free(aux_bufr);
                    return -1;
                }
                if(bufr[i] == 0x5d){
                    i++;
                    read(fd, &bufr[i], 1);
                    if(bufr[i] == 0x7c){
                        bufr[i-1] = 0x5c;
                        cont++;
                    }
                    if(bufr[i] == 0x7d){
                        bufr[i-1] = 0x5d;
                        cont++;
                    }
                    i--;
                }
        
                else
                {

                    bufr = aux_bufr;
                }
                // guarda no packet antes da leitura porque ao voltar para o inicio do primeiro while, ocorre a chamada da função read()
                read(fd, &bufr[i], 1);
                printf("%c valor de i:%d\n", bufr[i], i);
                cont++;
                if (bufr[i] == 0x5c) break; //flag final do packet
                i++;
            }
            printf(" guardado no buffer com sucesso\n");
            STATE = 5;
            break;
        case 5:
            printf("STATE: %d\n", STATE);
            printf("Dados do buffer:");
            printf("valor do i:%d\n", i);
            for (int j = 4; j < (i - 1); j++) // -1 para não inclui BCC2
            {                                 // bufr[i] = 0x5c neste momento
            printf("%c",bufr[j]);

                BCC2 = BCC2 ^ bufr[j];
                printf("Valor atual de BCC2:%02x\n", BCC2);
            }
            printf("Valor em hexadecimal de BCC2:%02x e de bufr[i - 1]:%02x", BCC2, bufr[i - 1]);
            if (BCC2 == bufr[i - 1])
            {
                for (int j = 0; j <= i; j++)
                {
                    *packet = bufr[j];
                    if (j != i)
                        packet++;
                    printf("%c", *packet);
                }
                STOP = TRUE;
                free(bufr); 
                printf(" Packet recebido com sucesso\n");
            }
            else
            {
                printf(" Erro na receção do packet\n");
                return -1;
            }
            break;

        default:
            break;
        }

        //STOP & WAIT PROTOCOL
        /*
        write(fd, ACK, 1);
!!!!!!!!
        */


    }
    return cont;
}
//TERMINA LLREAD


//COMECA O LLCLOSE
int llclose(linkLayer connectionParameters, int showStatistics)
{
    // define os parametros associados
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    

    /*Apartir deste momento, o terminal está corretamente configurado para manipulação dos dados
     com as configurações da estrutura newtio*/
    printf("New termios structure set\n");
    role = connectionParameters.role;
    unsigned char buffer;
    int STOP = FALSE, STATE = 0, i = 0, j=0;

    if (role == TRANSMITTER && j==0)
    { // Só para o noncanonical no final porque refere-se ao envio do AK
        i = 0;
        while (i <= 4)
        {

            if (i == 0)
                buffer = 0x5c;
            if (i == 1)
                buffer = 0x01;
            if (i == 2)
                buffer = 0x0A;
            if (i == 3)
                buffer = 0x01 ^ 0x0A;
            if (i == 4)
                buffer = 0x5c;
            printf("%02x\n", buffer);

            if (res = write(fd, &buffer, 1) < 0)
            {
                perror("Erro no envio do DISC");
                return -1;
            }
            buffer++;

            i++;
        }
        printf("DISC enviado com sucesso\n");
    }

    i = 0;
    unsigned char XOR;
    while (i <= 4)
    {

        res = read(fd, &buffer, 1); // Aqui, os programas encontra-se no estado 0 da máquina de estados a espera de receber alguma coisa do outro programa
        printf("%d \n", res);
        if (res < 0)
        {
            perror("Erro de leitura");
            return -1;
        }
        printf("%02x\n", buffer);

        if (STATE == 0)
        {
            printf("STATE: %d\n", STATE);
            STATE++;
        }

        switch (STATE)
        {
        case 1:
            printf("STATE: %d\n", STATE);
            if (buffer == 0x5c)
                break; // loop de flags 0x5c

            if (role == TRANSMITTER )
            {
                if (buffer == 0x03)
                    STATE++;
                else
                    STATE = 0;
            }

            if (role == RECEIVER)
            {
                if (buffer == 0x01)
                    STATE++;
                else
                    STATE = 0;
            }
            break;

        case 2:
            printf("STATE: %d\n", STATE);
            if (role == TRANSMITTER)
            {
                if (buffer == 0x0A)
                {
                    STATE = 3;
                    break;
                }
            }

            if (role == RECEIVER && j==0)
            {
                if (buffer == 0x0A)
                {
                    STATE = 3;
                    break;
                }
            }

            if (role == RECEIVER && j==1)
            {
                if (buffer == 0x06)
                {
                    STATE = 3;
                    break;
                }
            }

            if (buffer == 0x5c)
                STATE = 1;
            else
                STATE = 0;
            break;

        case 3:
            printf("STATE: %d\n", STATE);
            if (role == TRANSMITTER)
            {
                XOR = 0x03 ^ 0x0A;
                if (buffer == XOR)
                {
                    STATE = 4;
                    break;
                }
            }

            if (role == RECEIVER && j==0)
            {
                XOR = 0x01 ^ 0x0A;
                if (buffer == XOR)
                {
                    STATE = 4;
                    break;
                }
            }

            if (role == RECEIVER && j==1)
            {
                XOR = 0x01 ^ 0x06;
                if (buffer == XOR)
                {
                    STATE = 4;
                    break;
                }
            }

            if (buffer == 0x5c)
                STATE = 1;
            else
                STATE = 0;
            break;

        case 4:
            printf("STATE: %d\n", STATE);

            if (buffer == 0x5c)
                STOP = TRUE;
            else
                STATE = 0;
            break;

        default:
            break;
        }
        i++;
    }

    if (STOP == TRUE && role == RECEIVER)
    { // Só para o noncanonical no final porque refere-se ao envio do UA
        i = 0;
        while (i <= 4)
        {

            if (i == 0)
                buffer = 0x5c;
            if (i == 1)
                buffer = 0x03;
            if (i == 2)
                buffer = 0x0A;
            if (i == 3)
                buffer = 0x03 ^ 0x0A;
            if (i == 4)
                buffer = 0x5c;
            printf("%02x\n", buffer);

            if (res = write(fd, &buffer, 1) < 0)
            {
                perror("Erro no envio do DISC");
                return -1;
            }
            i++;
        }
        j=1;
        printf("DISC enviado com sucesso\n");
    }
    else if (role == RECEIVER)
    {
        printf("DISC não enviado\n");
    }
    if (STOP == TRUE && role == TRANSMITTER)
        printf("DISC recebido com sucesso\n");
    else if (role == TRANSMITTER)
        printf("DISC não recebido\n");

    /*o terminal volta as configurações originais devido ao facto de certas aplicações
    não definem as proprias configurações de terminal, por isso é importante voltar as definições originais*/

    if (role == TRANSMITTER && j==1)
    { // Só para o noncanonical no final porque refere-se ao envio do AK
        i = 0;
        while (i <= 4)
        {

            if (i == 0)
                buffer = 0x5c;
            if (i == 1)
                buffer = 0x01;
            if (i == 2)
                buffer = 0x06;
            if (i == 3)
                buffer = 0x01 ^ 0x06;
            if (i == 4)
                buffer = 0x5c;
            printf("%02x\n", buffer);

            if (res = write(fd, &buffer, 1) < 0)
            {
                perror("Erro no envio do UA");
                return -1;
            }
            buffer++;

            i++;
        }
        printf("UA enviado com sucesso\n");
    }

    close(fd);
    return 0;

}

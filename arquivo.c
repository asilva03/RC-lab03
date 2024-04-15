#include <stdio.h> 
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
/*Atenção que neste */

int main(){
    char buffer[2000]= {'\0'};
    char msg[50] = {'\0'};
    printf("Introduza uma mensagem a ser guardada em texto: ");
    fgets(msg, 50, stdin);
    int k = strcspn(msg, "\n");
    msg[k] = '\0';
    int fd = open("arquivo.txt", O_RDWR | O_CREAT );

    printf("fd = %d\n", fd);
    if(fd == -1){
        perror("Erro");
        return 0;
    }
    puts(msg);
    write(fd, msg, k);
    /*mgs tem sempre mais um espaço do que o que realmente vai ser escrito dentro
    dentro do arquivo, porque o retorno da função strcspn é o comprimento até 
    à primeira posição do caracter da string a se comparar, não incluso.
    */
    lseek(fd, 0, SEEK_SET); // inicialização de msg
    int j = strlen(msg);
    printf("%d\n", j);
    int i = read(fd, buffer, k);
    printf("A mensagem escrita no arquivo é: %s. bytes: %d\n", buffer,i);
    close(fd);
    return 0;
}

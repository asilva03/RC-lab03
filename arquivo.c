#include <stdio.h> 
#include <unistd.h>
#include <fcntl.h>

int main(){
    char buffer[2000];
    char msg[50] = "Olá Mundo";
    int fd = open("arquivo.txt", O_RDWR | O_CREAT);
    close(fd);
    fd = open("arquivo.txt", O_RDWR | O_CREAT);
    printf("fd = %d\n", fd);
    if(fd == -1){
        perror("Erro");
        return 0;
    }
    write(fd, msg, sizeof(msg));
    lseek(fd, 0, SEEK_SET); //ou rewind(fd)
    read(fd, buffer, sizeof(buffer));
    printf("A mensagem escrita no arquivo é: %s", buffer);
    close(fd);
    return 0;
}

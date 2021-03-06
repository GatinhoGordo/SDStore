#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

#define mainFIFO "../tmp/mainFIFO"
#define server_client_fifo "../tmp/server_client_"
#define client_server_fifo "../tmp/client_server_"
#define messageSize 256
#define statusSize 4096

void println(char *str){

    char nl = 10;
    write(1,str,strlen(str));
    write(1,&nl,1);

}

// O pedido será do tipo "pid pedido arg arg..."
int main(int argc,char *argv[]){

    int pid = (int) getpid();
    char pedido[messageSize];
    char fifoWrite[messageSize];
    char fifoRead[messageSize];
    int mainFIFOfd = open(mainFIFO,O_WRONLY);
    int status=0;
    
    //Construir o pedido com os argumentos fornecidos pelo cliente
    memset(pedido,0,sizeof(pedido));
    sprintf(pedido,"%d ",pid);
    for(int i=1;i<argc;i++){
        strcat(pedido,argv[i]);
        strcat(pedido," ");
    }
    //printf("->%s\n",pedido);

    //Confirmar os argumentos e enviar o pedido
    if(argc<2){

        println("Argumentos insuficientes");
        return 1;
    }
    else{

        if(strcmp("status",argv[1])==0){
            
            if(argc != 2){

                println("Argumentos inválidos");
                return 1;
            }

            else status=1;

        }

        else if(strcmp("proc-file",argv[1])==0){

            if(argc < 5){

                println("Argumentos inválidos");
                return 1;
            }
        }

        else println("Operação inválida");

        if(write(mainFIFOfd,pedido,sizeof(pedido))<0){

            perror("Erro na escrita");
            return 1;
        }
    }

    close(mainFIFOfd);
    
    //Preparar e abrir os FIFOs "privados"
    memset(fifoRead,0,sizeof(fifoRead));
    memset(fifoWrite,0,sizeof(fifoWrite));
    sprintf(fifoRead,"%s%d",server_client_fifo,(int) pid);
    sprintf(fifoWrite,"%s%d",client_server_fifo,(int) pid);

    //Tenho que arranjar uma forma de saber se o server já criou os fifos
    sleep(1);

    //É um pedido de status
    if(status){

        //int fdW = open(fifoWrite,O_WRONLY);
        int fdR = open(fifoRead,O_RDONLY);
        if(fdR==-1){
            perror("Erro ao abrir o pipe");
            exit(-1);
        }

        char resposta[statusSize];
        int respostaSize = read(fdR,resposta,sizeof(resposta));
        close(fdR);

        if(respostaSize>0){

            println(resposta);

        }
        else{

            perror("Erro na Leitura: ");
            exit(-1);

        }

    }

    //É um pedido de proc-file
    else{

        int fdR = open(fifoRead,O_RDONLY);
        if(fdR==-1){
            perror("Erro ao abrir o pipe");
            exit(-1);
        }

        char resposta[messageSize];
        int respostaSize;
        while((respostaSize = read(fdR,resposta,sizeof(resposta)))>0){

            resposta[respostaSize] = 0;
            println(resposta);
        
        }

        println("Conexão terminada\n");

    }


    return 0;

}
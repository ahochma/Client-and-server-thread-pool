#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <strings.h>
#include <netinet/in.h>
#include <signal.h>

void error(char *msg) {
    perror(msg);
    exit(0);}

int main(int argc, char ** argv) {
    //Create new pipe
    int fd[2];
    pipe(fd);
    //Create a son process to accept input from user
    pid_t pid = fork();
    if (pid == 0) {
        close(fd[0]);
        char buffer[256];
        while (1){
            usleep(2000);
            printf ("> ");
            scanf("%[^\n]%*c", buffer);
            write(fd[1], buffer, 256);
            memset(buffer,'0', 256);
        }
    }
    else {
        close(fd[1]);
        int sockfd, portno, n, r;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        char buffer[256];
        char server_message[256];
        if (argc < 3) {
            fprintf(stderr,"usage %s hostname port\n", argv[0]);
            exit(0);}
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            error("ERROR opening socket");}
        portno = atoi(argv[2]);
        server = gethostbyname(argv[1]);
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);}
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr_list[0], (char *) 	&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(portno);
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
            error("ERROR connecting");}
       
        while(1){
            read(fd[0], buffer, sizeof(buffer));
            if (write(sockfd, buffer, sizeof(buffer)) < 0){
                puts("Failed Sending\n");
                return 1;
            }
            char GradeList[256] = "GradeList";
            char done[256] = "Done";
            if (strcmp(GradeList, buffer) == 0){
                while(1){
                    int res = 0;
                    res = read(sockfd, server_message, 256);
                    if (strcmp(server_message, done) == 0){
                        char zero[256] = {'0'};
                        strcpy(server_message, zero);
                        break;
                    }
                    printf("%s", server_message);
                    char z[256] = {'0'};
                    strcpy(server_message, z);
                }
            } else {
                read(sockfd, server_message, 256);
                char bye[256] = "BYE";
                if (strcmp(server_message, bye) == 0) {
                    close(sockfd);
                    close(fd[0]);
                    char zero[256] = {'0'};
                    strcpy(buffer, zero);
                    close(sockfd);
                    kill(pid, SIGKILL);
                    exit(0);
                }
                printf("%s", server_message);
                fflush(stdout);
                bzero(buffer, 256);
                bzero(server_message, 256);
            }



        }

    }
    return 0;

}
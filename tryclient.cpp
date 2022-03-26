#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<iostream>
#define cout std::cout
#define cin std::cin
#define endl std::endl
#define SENDPORT 3000
#define RECPORT 3001
int main(){
    int recfd,clientfd;
    struct sockaddr_in rec_addr,client_addr;
    recfd = socket(AF_INET, SOCK_STREAM, 0);
    rec_addr.sin_family = AF_INET;
    rec_addr.sin_port = htons(RECPORT);
    rec_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(rec_addr.sin_zero), '\0', 8);
    if(bind(recfd, (struct sockaddr *)&rec_addr, sizeof(struct sockaddr)) < 0){ 
        perror("Bind Failed for rec fd\n"); 
        exit(0); 
    } else{
        cout<<"Bound rec fd:)\n";
    } 
    if(listen(recfd,20)){ 
        perror("Listening Failed\n"); 
        exit(0); 
    } else{
        cout<<"Listened :)\n";
    }
    int sendfd;
    struct sockaddr_in target_addr;
    sendfd = socket(AF_INET, SOCK_STREAM, 0); 
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(RECPORT);
    target_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    memset(&(target_addr.sin_zero), '\0', 8); 
    if(connect(sendfd, (struct sockaddr *)&target_addr, sizeof(struct sockaddr)) < 0){ 
        perror("Connection Failed\n"); 
        exit(0); 
    } 
    int sin_size=sizeof(struct sockaddr_in);
    clientfd=accept(recfd,(struct sockaddr *)&client_addr, (socklen_t*)&sin_size);
    char *msg = "Beej was here!";
    int len, bytes_sent;
    len = strlen(msg);
    bytes_sent = send(sendfd, msg, len, 0);
    cout<<bytes_sent<<endl;
    char buff[1000];
    int bytes_rec=recv(clientfd,buff,999, 0);
    cout<<bytes_rec<<endl;
    cout<<buff<<endl;
    shutdown(recfd,2);
    shutdown(sendfd,2);
}

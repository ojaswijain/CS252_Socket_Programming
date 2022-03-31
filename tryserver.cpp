#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<iostream>
#define cout std::cout
#define cin std::cin
#define endl std::endl
#define RECPORT 2500
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
    int sin_size=sizeof(struct sockaddr_in);
    clientfd=accept(recfd,(struct sockaddr *)&client_addr, (socklen_t*)&sin_size);
    char buff[1000];
    int bytes_rec=recv(clientfd,buff,999, 0);
    cout<<bytes_rec<<endl;
    cout<<buff<<endl;
}

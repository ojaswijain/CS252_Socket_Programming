#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<iostream>
#define cout std::cout
#define cin std::cin
#define endl std::endl
#define MYSOURCEPORT 2709
#define MYDESTPORT 2709
int main(){
    int sockfd,new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in dest_addr;
    struct sockaddr_in their_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(2709);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8); 
    if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0){ 
        perror("Bind Failed\n"); 
        exit(0); 
    } else{
        cout<<"Bound :)\n";
    }
    if(listen(sockfd,20)){ 
        perror("Litening failed Failed\n"); 
        exit(0); 
    } else{
        cout<<"Listened :)\n";
    }
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(2709);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(dest_addr.sin_zero), '\0', 8); 
    // listen(sockfd,20);
    int sin_size=sizeof(struct sockaddr_in);
    cout<<"here\n";
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, (socklen_t*)&sin_size);
    cout<<"here\n";
    printf("%s", inet_ntoa(their_addr.sin_addr));
    if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) < 0){ 
        perror("Connection Failed\n"); 
        exit(0); 
    } 
    char *msg = "Beej was here!",*buff;;
    int len, bytes_sent;
    len = strlen(msg);
    bytes_sent = send(sockfd, msg, len, 0);
    int bytes_rec=recv(new_fd,buff,100, 0);
    cout<<bytes_rec<<endl;
    shutdown(new_fd, 2);
    
}
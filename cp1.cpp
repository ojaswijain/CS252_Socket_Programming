#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;
int main(int argc, char *argv[]){
    int sno,id,myport,numneigh,*neigh_sno,*neigh_ports;
    ifstream config;
    config.open(argv[2]);
    string in_line,T;
    getline(config, in_line);
    stringstream X(in_line);
    getline(X, T, ' ');
    sno=stoi(T);
    getline(X, T, ' ');
    myport=stoi(T);
    getline(X, T, ' ');
    id=stoi(T);
    getline(config, in_line);
    numneigh=stoi(in_line);
    neigh_sno=new int[numneigh];
    neigh_ports=new int[numneigh];
    getline(config, in_line);
    stringstream Y(in_line);
    int i=0;
    while(i<numneigh){
        getline(Y, T,' ');
        neigh_sno[i]=stoi(T);
        getline(Y, T,' ');
        neigh_ports[i]=stoi(T);
        i++;
    }
    config.close();
    int recfd,neighfd;
    struct sockaddr_in rec_addr,neigh_addr;
    recfd = socket(AF_INET, SOCK_STREAM, 0);
    rec_addr.sin_family = AF_INET;
    rec_addr.sin_port = htons(myport);
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
        cout<<"Listening :)\n";
    }
    int sendfd[numneigh];
    fd_set master,read_fds;
    int fdmax,nbytes;
    char buf[256];
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(recfd, &master);
    fdmax = recfd;
    for(int i=0;i<numneigh;i++){
        sendfd[i]=socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in target_addr;
        target_addr.sin_family = AF_INET;
        target_addr.sin_port = htons(neigh_ports[i]);
        target_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        memset(&(target_addr.sin_zero), '\0', 8); 
        cout<<neigh_ports[i]<<endl;
        while(connect(sendfd[i],(struct sockaddr *)&target_addr, sizeof(struct sockaddr)) < 0){}
        cout<<"Connected\n";
        fdmax=max(fdmax,sendfd[i]);
    }
    for(;;){
        read_fds=master;
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(1);
        }
        for(i=0;i<=fdmax;i++){
            if (FD_ISSET(i,&read_fds)){
                if (i==recfd){
                    int sin_size=sizeof(struct sockaddr_in);
                    if(neighfd=accept(recfd,(struct sockaddr *)&neigh_addr, (socklen_t*)&sin_size)==-1){
                        perror("accept");
                    }else{
                        if((nbytes=send(i, "try", int len, int flags)<= 0){
                            if (nbytes == 0) {
                                printf("selectserver: socket %d hung up\n", i);
                            } else {
                                perror("recv");
                            }
                            close(i); // bye!
                            FD_CLR(i, &master);
                        }else{
                            cout<<nbytes;
                        }
                        FD_SET(neighfd, &master_read);
                        if (neighfd>fdmax){
                            fdmax = neighfd;
                        }  
                    }
                }else{
                    
                }
            }
        }
    }

}

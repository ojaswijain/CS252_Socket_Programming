#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <dirent.h>
using namespace std;

int main(int argc, char *argv[]){
    int no,id,port,nbrs;
    freopen(argv[0],"r",stdin);
    cin>>no>>id>>port;
    cin>>nbrs;

    int nos[nbrs],ports[nbrs];
    for(int i=0;i<nbrs;i++){
        cin>>nos[i]>>ports[i];
    }

    // string path = argv[1];
    // for (const auto & entry : fs::directory_iterator(path))
    //     cout <<entry.path()<<endl;

    DIR *pDIR;
    struct dirent *entry;
    if( pDIR=opendir(argv[1])){
        while(entry = readdir(pDIR)){
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                cout << entry->d_name <<endl;
            }
        closedir(pDIR);
    }

    int sendfd,recfd;

    struct sockaddr_in send_addr,rec_addr,nbr_addr;
    sendfd = socket(AF_INET, SOCK_STREAM, 0);
    recfd = socket(AF_INET, SOCK_STREAM, 0);

    rec_addr.sin_family = AF_INET;
    rec_addr.sin_port = htons(port);
    rec_addr.sin_addr.s_addr = INADDR_ANY;

    memset(&(rec_addr.sin_zero), '\0', 8);

    if(bind(recfd, (struct sockaddr *)&rec_addr, sizeof(struct sockaddr)) < 0) exit(0); 

    send_addr.sin_family = AF_INET;
    send_addr.sin_port = 0;
    send_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(send_addr.sin_zero), '\0', 8);

    if(listen(recfd,20)) exit(0);

    for(int i=0;i<nbrs;i++){
        struct sockaddr_in target_addr;
        target_addr.sin_family = AF_INET;
        target_addr.sin_port = htons(ports[i]);
        target_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        memset(&(target_addr.sin_zero), '\0', 8); 
        while(connect(sendfd,(struct sockaddr *)&target_addr, sizeof(struct sockaddr)) < 0){}
        //get unique ID 
        cout<<"Connected to "<<nos[i]<<" with unique-ID "<<123<<" on port "<<ports[i]<<endl;
    }

    if(listen(recfd,20)) exit(0);
}

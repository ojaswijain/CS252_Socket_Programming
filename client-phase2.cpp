#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <dirent.h>
#include <bits/stdc++.h>
int INTMAX=1000000007;

void send_response(){

}

using namespace std;

int main(int argc, char *argv[]){
    int sno,id,myport,nbrs;
    freopen(argv[2],"r",stdin);
    cin>>sno>>myport>>id;
    cin>>nbrs;
    int nos[nbrs],ports[nbrs];
    for(int i=0;i<nbrs;i++){
        cin>>nos[i]>>ports[i];
    }
    int nfiles; cin>>nfiles;
    vi files(nfiles);
    for(int i=0;i<nfiles;i++){
        cin>>files[i];
    }

    set <string> myfiles;
    set <string> connects;
    DIR *pDIR;
    struct dirent *entry;
    if( pDIR=opendir(argv[1])){
        while(entry = readdir(pDIR)){
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                myfiles.insert(entry->d_name);
            }
        closedir(pDIR);
    }

    int recfd,neighfd;
    struct sockaddr_in rec_addr,neigh_addr;
    recfd = socket(AF_INET, SOCK_STREAM, 0);
    rec_addr.sin_family = AF_INET;
    rec_addr.sin_port = htons(myport);
    rec_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(rec_addr.sin_zero), '\0', 8);
    int yes=1;
    if (setsockopt(recfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    if(bind(recfd, (struct sockaddr *)&rec_addr, sizeof(struct sockaddr)) < 0){ 
        perror("binf"); 
        exit(0); 
    } 
    if(listen(recfd,20)){ 
        perror("listen"); 
        exit(0); 
    }
    int sendfd[nbrs];
    fd_set master,read_fds;
    int fdmax,nbytes,n_bytes;
    //change buffer size
    char buf[256];
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(recfd, &master);
    fdmax = recfd;
    map <int,int> nos_map;
    map <int, int> port_map;
    map <string, int> filemap;
    for(auto f: myfiles){
        filemap[f]=INTMAX;
    }
    for(int i=0;i<nbrs;i++){
        sendfd[i]=socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in target_addr;
        target_addr.sin_family = AF_INET;
        target_addr.sin_port = htons(ports[i]);
        target_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        memset(&(target_addr.sin_zero), '\0', 8); 
        while(connect(sendfd[i],(struct sockaddr *)&target_addr, sizeof(struct sockaddr)) < 0){}
        FD_SET(sendfd[i], &master);
        port_map[sendfd[i]]=ports[i];
        nos_map[sendfd[i]]=nos[i];
        fdmax=max(fdmax,sendfd[i]);
    }
    int count=0;
    for(;;){
        read_fds=master;
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(1);
        }
        for(int i=0;i<=fdmax;i++){
            if (FD_ISSET(i,&read_fds)){
                if (i==recfd){
                    int sin_size=sizeof(struct sockaddr_in);
                    neighfd=accept(recfd,(struct sockaddr *)&neigh_addr, (socklen_t*)&sin_size);
                    if(neighfd==-1){
                        perror("accept");
                    }else{
                        nbytes=send(neighfd, to_string(id).c_str(), strlen(to_string(id).c_str()), 0);
                        if(nbytes<= 0){
                            if (nbytes == 0) {
                                printf("selectserver: socket %d hung up\n", i);
                                shutdown(neighfd,2);
                            } else {
                                perror("send");
                            }
                        }
                        for(auto f: files){
                            string filename='$'+f;
                            n_bytes=send(neighfd, to_string(filename).c_str(), strlen(to_string(filename).c_str()), 0);
                        }
                    }
                }else{
                    // cout<<"here\n";
                    nbytes=recv(i, buf, sizeof(buf), 0);
                    if (nbytes == 0) {
                        printf("selectserver: socket %d hung up\n", i);
                        shutdown(i,2);
                    }
                    else if(nbytes<0){
                        perror("recv");
                    }
                    else{ 
                        string s=buf;
                        if(s[0]=='$'){
                            s.erase(0,1);
                            if(myfiles.count(s)==1){
                                string ye='#'+s;
                                n_bytes=send(i, to_string(ye).c_str(), strlen(to_string(ye).c_str()), 0);
                            }
                        }

                        if(s[0]=='#'){
                            s.erase(0,1);
                            filemap[s]=min(filemap[s],i);
                        }

                        else{
                            cout<<"Connected to " <<nos_map[i]<< " with unique-ID "<<s<<" on port "<<port_map[i]<<"\n";
                            connects.insert(s);
                            if(connects.size()>=nbrs) break;
                        }
                    }
                }
            }
        }
    }
    for (auto *it=filemap.begin(); it!=filemap.end();it++){
        if(it->second==INTMAX) cout<<"Found "<<it->first<<" at 0 with MD5 0 at depth 0"<<endl;
        else cout<<"Found "<<it->first<<" at "<<it->second<<" with MD5 0 at depth 1"<<endl;
    }
}

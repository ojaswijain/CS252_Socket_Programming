// sudo apt install libssl-dev
// g++ client-phase3.cpp -lcrypto

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <openssl/md5.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <bits/stdc++.h>
// #include <thread>
int INTMAX=1000000007;
using namespace std;

void calcMD5(string filename, unsigned char * result){
    ifstream file(filename, ifstream::binary);
    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    char buf[1024 * 16];
    while (file.good()){
        file.read(buf, sizeof(buf));
        MD5_Update(&md5Context, buf, file.gcount());
    }
    MD5_Final(result, &md5Context);
}

void printMD5(unsigned char * hash){
    for(int i=0;i<MD5_DIGEST_LENGTH;i++) printf("%02x",hash[i]);
}

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
    vector<string> files(nfiles);
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
    vector<int> recfds;
    fd_set master,read_fds;
    int fdmax,nbytes,n_bytes;
    //change buffer size
    char buf[1024];
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(recfd, &master);
    fdmax = recfd;
    map <int,int> nos_map;
    map <int, int> port_map;
    map <int, int> i_map;
    map <string, int> filemap;
    map <string, int> pendingmap;
    map <int,int> IDs;
    for(auto f: files){
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
        i_map[sendfd[i]]=i;
        fdmax=max(fdmax,sendfd[i]);
    }
    int count=nbrs;
    bool tick=0;
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
                        FD_SET(neighfd, &master);
                        fdmax=max(fdmax,neighfd);
                        recfds.push_back(neighfd);
                        nbytes=send(neighfd, ("&"+to_string(id)).c_str(), strlen(("&"+to_string(id)).c_str()), 0);
                        if(nbytes<= 0){
                            if (nbytes == 0) {
                                // printf("selectserver: socket %d hung up\n", i);
                                FD_CLR(i,&master);
                                shutdown(neighfd,2);
                            } else {
                                perror("send");
                            }
                        }
                    }
                }else{
                    nbytes=recv(i, buf, sizeof(buf), 0);
                    if (nbytes == 0) {
                        // printf("selectserver: socket %d hung up\n", i);
                        FD_CLR(i,&master);
                        shutdown(i,2);
                    }
                    else if(nbytes<0){
                        perror("recv");
                    }
                    else{ 
                        string s=buf;
                        bzero(buf,sizeof(buf));
                    // memset( buf, '\0', sizeof(char)*1025 );
                        if(s[0]=='&'){
                            // cout<<s<<endl;
                            s.erase(0,1);
                            int x=stoi(s);
                            IDs[i]=x;
                            cout<<"Connected to " <<nos_map[i]<< " with unique-ID "<<x<<" on port "<<port_map[i]<<"\n";
                            connects.insert(s);
                            if(connects.size()>=nbrs){
                                break;
                            }
                        }else{
                            s.erase(0,1);
                            pendingmap[s]=i;
                        }
                    }
                }
            }
        }
        if(connects.size()>=nbrs) break;
    }
    // cout<<"here\n";
    bzero(buf,sizeof(buf));
                    // memset( buf, '\0', sizeof(char)*1025 );
    int crntfile=0;
    count=nbrs;
    for(int i=0;i<nbrs;i++){
        string filename='$'+files[0];
        n_bytes=send(sendfd[i], filename.c_str(), strlen(filename.c_str()), 0);
        // cout<<n_bytes<<endl;
    }
    int confirm=0;
    for (auto it=pendingmap.begin(); it!=pendingmap.end();it++){
        string s=it->first;
        int soc=it->second;
        if(myfiles.count(s)==1){
            string ye='#'+s;
            n_bytes=send(soc, ye.c_str(), strlen(ye.c_str()), 0);
            cout<<n_bytes<<endl;
        }else{
            string ye="%"+s;
            n_bytes=send(soc, ye.c_str(), strlen(ye.c_str()), 0);
        }
    }
    FD_CLR(recfd,&master);
    for(;;){
        read_fds=master;
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(1);
        }
        for(int i=0;i<=fdmax;i++){
            if (FD_ISSET(i,&read_fds)){
                // cout<<i<<" ";
                nbytes=recv(i, buf, sizeof(buf), 0);
                // cout<<buf<<endl;
                if (nbytes == 0) {
                    // printf("selectserver: socket %d hung up\n", i);
                    FD_CLR(i,&master);
                    shutdown(i,2);
                }
                else if(nbytes<0){
                    perror("recv");
                }
                else{ 
                    string s=buf;
                    // cout<<s<<endl;
                    bzero(buf,sizeof(buf));
                    // memset( buf, '\0', sizeof(char)*1025 );
                    if(s[0]=='$'){
                        // cout<<s<<endl;
                        s.erase(0,1);
                        if(myfiles.count(s)==1){
                            string ye='#'+s;
                            n_bytes=send(i, ye.c_str(), strlen(ye.c_str()), 0);

                        }else{
                            string ye="%"+s;
                            n_bytes=send(i, ye.c_str(), strlen(ye.c_str()), 0);
                        }
                    }else if(s[0]=='#'){
                        // cout<<"Found "<<s<<" "<<IDs[i]<<endl;
                        s.erase(0,1);
                        filemap[s]=min(filemap[s],i);
                        count--;
                        if(count==0){
                            break;
                        }
                    }else if(s[0]=='%'){
                        s.erase(0,1);
                        count--;
                        if(count==0){
                            break;
                        }
                    }else if(s[0]=='&'){
                        // cout<<s<<endl;
                        confirm++;
                        // cout<<crntfile<<" "<<count<<endl;
                        // count--;
                    }
                }
            }
        }
        if(count==0 || crntfile>=nfiles){
            tick=0;
            count=nbrs;
            crntfile++;
            // cout<<confirm<<" "<<crntfile<<endl;
            if(confirm==nbrs && crntfile>nfiles){
                count=0;
                break;
            }else if(crntfile==nfiles){
                count=0;
                // cout<<"here\n";
                for(int i=0;i<nbrs;i++){
                    string star="&"+to_string(id);
                    n_bytes=send(sendfd[i], star.c_str(), strlen(star.c_str()), 0);
                }
                if(confirm==nbrs){
                    break;
                }
            }else if(crntfile<nfiles){
                for(int i=0;i<nbrs;i++){
                    string filename='$'+files[crntfile];
                    // cout<<filename<<endl;
                    n_bytes=send(sendfd[i], filename.c_str(), strlen(filename.c_str()), 0);
                    // cout<<nbytes<<endl;
                }
            }
        }
    }
    for (auto it=filemap.begin(); it!=filemap.end();it++){
        if(it->second==INTMAX) cout<<"Found "<<it->first<<" at 0 with MD5 0 at depth 0"<<endl;
        else cout<<"Found "<<it->first<<" at "<<IDs[it->second]<<" with MD5 0 at depth 1"<<endl;
    }
    map<int,set<string>> port_files;
    int foundfiles=0;
    for (auto it=filemap.begin(); it!=filemap.end();it++){
        if(it->second!=INTMAX){
            foundfiles++;
            port_files[it->second].insert(it->first);
            cout<<it->second<<" "<<it->first<<endl;
        }

    }
    
    // string filecontent="";
    for (auto it=port_files.begin(); it!=port_files.end();it++){
        string s=*((it->second).begin());
        int soc=it->first;
        string filename='$'+s;
        n_bytes=send(soc, filename.c_str(), strlen(filename.c_str()), 0);
        // cout<<n_bytes<<endl;
    }
    // int leftfiles=foundfiles;
    for(;;){
        read_fds=master;
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(1);
        }
        for(int i=0;i<=fdmax;i++){
            if (FD_ISSET(i,&read_fds)){
                nbytes=recv(i, buf,1024, 0);
                if (nbytes == 0) {
                    // printf("selectserver: socket %d hung up\n", i);
                    FD_CLR(i,&master);
                    shutdown(i,2);
                }
                else if(nbytes<0){
                    perror("recv");
                }
                else{ 
                    // buf[1024]='\0';
                    string s=buf;
                    // memset( buf, '\0', sizeof(char)*1025 );
                    if(s[0]=='$'){
                        s.erase(0,1);
                        FILE *picture;
                        picture = fopen(("sample-data/files/client"+to_string(sno)+"/"+s).c_str(), "rb");
                        fseek(picture, 0, SEEK_SET);
                        printf("Sending Picture as Byte Array\n");
                        char send_buffer[1024]={0};
                        int k;
                        while ((k = fread(send_buffer, 1, sizeof(send_buffer), picture)) > 0){
                            if (send(i, send_buffer, k, 0) == -1)
                            {
                                perror("[-]Error in sending file.");
                                exit(1);
                            }
                            // cout<<send_buffer<<flush;
                            bzero(send_buffer, 1024);
                        }
                        cout<<"here\n";
                        // fclose(picture);
                    }else{
                        string name =*((port_files[i]).begin());
                        FILE* myfile=fopen(name.c_str(),"wb");
                        // s=buf;
                        int k=nbytes;
                        while(k>0){
                            cout<<k<<endl;
                            // cout<<s.length()<<endl;
                            // buf[1024]='\0';
                            fwrite(buf,1,k,myfile);
                            fflush(myfile);
                            // cout<<s<<flush;
                            bzero(buf,sizeof(buf));
                            k=recv(i, buf, 1024, 0);
                            // s=buf;
                        }
                        fclose(myfile);
                        port_files[i].erase(name);
                        if(!port_files[i].empty()){
                            name=*((port_files[i]).begin());
                            string filename='$'+name;
                            n_bytes=send(i, filename.c_str(), strlen(filename.c_str()), 0);
                        }
                    }
                    bzero(buf,sizeof(buf));
                }
            }
        }
    }
}

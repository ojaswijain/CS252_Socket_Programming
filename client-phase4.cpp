#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <dirent.h>
#include <bits/stdc++.h>
#include <chrono>
// #include <thread>
int INTMAX=1000000007;
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
    vector<string> files;
    for(int i=0;i<nfiles;i++){
        string x;
        cin>>x;
        files.push_back(x);
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
    char buf[256];
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(recfd, &master);
    fdmax = recfd;
    map <int,int> nos_map;
    map <int, int> port_map;
    // map <int, int> i_map;
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
        // i_map[sendfd[i]]=i;
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
                        memset( buf, '\0', sizeof(char)*256 );
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
    memset( buf, '\0', sizeof(char)*256 );
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
    int l1confirm=0;
    map<string,int> nbrpending;
    map<string,int> d2pending;
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
                    memset( buf, '\0', sizeof(char)*256 );
                    vector<string> next,file,yes,no,conf,l1,d2;
                    char k=s[0];
                    s.erase(0,1);
                    string const delims{"!$%&@#{"};
                    size_t beg, pos = 0;
                    while ((beg =s.find_first_not_of(delims, pos)) != string::npos){
                        pos=s.find_first_of(delims, beg + 1);
                        string temp=s.substr(beg, pos - beg);
                        switch(k){
                            case '@':
                                d2.push_back(temp);
                                break;
                            case '{':
                                l1.push_back(temp);
                                break;
                            case '!':
                                next.push_back(temp);
                                break;
                            case '$':
                                file.push_back(temp);
                                break;
                            case '%':
                                no.push_back(temp);
                                break;
                            case '#':
                                yes.push_back(temp);
                                break;
                            case '&':
                                conf.push_back(temp);
                                break;
                            default:
                                break;
                        }
                        k=s[pos];
                    }
                    for(auto a:d2){
                        d2pending[a]=i;
                    }
                    for(auto a:next){
                        nbrpending[a]=i;
                    }
                    for(auto a:file){
                        if(myfiles.count(a)==1){
                            string ye='#'+a;
                            n_bytes=send(i, ye.c_str(), strlen(ye.c_str()), 0);
                        }else{
                            string ye="%"+a;
                            n_bytes=send(i, ye.c_str(), strlen(ye.c_str()), 0);
                        }
                    }
                    for(auto a:yes){
                        filemap[a]=min(filemap[a],IDs[i]);
                        count--;
                    }
                    for(auto a:no){
                        count--;
                    }
                    for(auto a:conf){
                        confirm++;
                    }
                    for(auto a:l1){
                        l1confirm++;
                    }
                    if(count==0){
                        break;
                    }
                }
            }
        }
        if(count==0 || crntfile>=nfiles){
            tick=0;
            count=nbrs;
            crntfile++;
            if(confirm==nbrs && crntfile>nfiles){
                count=0;
                break;
            }else if(crntfile==nfiles){
                count=0;
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
                    n_bytes=send(sendfd[i], filename.c_str(), strlen(filename.c_str()), 0);
                }
            }
        }
    }
    int l2confirm=0;
    int numleft=0;
    deque<string> filesleft;
    map <string,pair<int,int>> d2files;
    map<string,pair<string,int>> nbrd2files;
    for(auto it=d2pending.begin();it!=d2pending.end();it++){
        string a=it->first;
        int p=a.find('^');
        string s1=a.substr(0,p);
        string s2=a.substr(p+1,a.length());
        if(myfiles.count(s1)==1){
            string ye=')'+a+"^"+to_string(id)+"^"+to_string(myport);
            n_bytes=send(it->second, ye.c_str(), strlen(ye.c_str()), 0);
        }else{
            string ye="("+a;
            n_bytes=send(it->second, ye.c_str(), strlen(ye.c_str()), 0);
        }  
    }
    for (auto it=nbrpending.begin(); it!=nbrpending.end();it++){
        string a=it->first;
        tick=false;
        int p=a.find('^');
        string s1=a.substr(0,p);
        string s2=a.substr(p+1,a.length());
        string filename='@'+a;
        for(int i=0;i<nbrs;i++){
            if(IDs[sendfd[i]]!=stoi(s2)){
                tick=true;
                n_bytes=send(sendfd[i], filename.c_str(), strlen(filename.c_str()), 0);
            }
        }
        if(!tick){
            string ye="("+a;
            n_bytes=send(it->second, ye.c_str(), strlen(ye.c_str()), 0);
        }else{
            // cout<<"first "<<a<<endl;
            nbrd2files[a]=make_pair(to_string(INT_MAX),0);
        }
    }
    for (auto it=filemap.begin(); it!=filemap.end();it++){
        if(it->second==INTMAX){
            numleft++;
            filesleft.push_back(it->first);
            d2files[it->first].first=INTMAX;
            d2files[it->first].second=0;
        }
    }
    if(numleft>0){
        for(int i=0;i<nbrs;i++){
            string filename='!'+filesleft[0]+'^'+to_string(id);
            n_bytes=send(sendfd[i], filename.c_str(), strlen(filename.c_str()), 0);
        }
    }else{
        for(int i=0; i<nbrs;i++){
            string a="{"+to_string(id);
            n_bytes=send(sendfd[i],a.c_str(), strlen(a.c_str()), 0);
        }
        numleft--;
    }
    count=nbrs;
    for(;;){
        read_fds=master;
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(1);
        }
        for(int i=0;i<=fdmax;i++){
            if (FD_ISSET(i,&read_fds)){
                nbytes=recv(i, buf, sizeof(buf), 0);
                if (nbytes == 0) {
                    FD_CLR(i,&master);
                    shutdown(i,2);
                }
                else if(nbytes<0){
                    perror("recv");
                }
                else{ 
                    string s=buf;
                    // cout<<s<<endl;
                    memset( buf, '\0', sizeof(char)*256 );
                    vector<string> d1,d2,yes,no,l1,l2;
                    char k=s[0];
                    s.erase(0,1);
                    string const delims{"!@(){}"};
                    size_t beg, pos = 0;
                    while ((beg =s.find_first_not_of(delims, pos)) != string::npos){
                        pos=s.find_first_of(delims, beg + 1);
                        string temp=s.substr(beg, pos - beg);
                        switch(k){
                            case '!':
                                d1.push_back(temp);
                                break;
                            case '@':
                                d2.push_back(temp);
                                break;
                            case ')':
                                yes.push_back(temp);
                                break;
                            case '(':
                                no.push_back(temp);
                                break;
                            case '{': 
                                l1.push_back(temp);
                                break;
                            case '}':
                                l2.push_back(temp);
                                break;
                            default:
                                break;
                        }
                        k=s[pos];
                    }
                    int p;
                    string s1,s2;
                    for(auto a:d1){
                        tick=false;
                        p=a.find('^');
                        s1=a.substr(0,p);
                        s2=a.substr(p+1,a.length());
                        string filename='@'+a;
                        for(int i=0;i<nbrs;i++){
                            if(IDs[sendfd[i]]!=stoi(s2)){
                                tick=true;
                                n_bytes=send(sendfd[i], filename.c_str(), strlen(filename.c_str()), 0);
                            }
                        }
                        if(!tick){
                            string ye="("+a;
                            n_bytes=send(i, ye.c_str(), strlen(ye.c_str()), 0);
                        }else{
                            // cout<<"first "<<a<<endl;
                            nbrd2files[a]=make_pair(to_string(INT_MAX),0);
                        }
                    }
                    for(auto a:d2){
                        p=a.find('^');
                        s1=a.substr(0,p);
                        s2=a.substr(p+1,a.length());
                        if(myfiles.count(s1)==1){
                            string ye=')'+a+"^"+to_string(id)+"^"+to_string(myport);
                            n_bytes=send(i, ye.c_str(), strlen(ye.c_str()), 0);
                        }else{
                            string ye="("+a;
                            n_bytes=send(i, ye.c_str(), strlen(ye.c_str()), 0);
                        }  
                    }
                    for(auto a:yes){
                        p=a.find('^');
                        s1=a.substr(0,p);
                        s2=a.substr(p+1,a.length());
                        int q=s2.find('^');
                        // cout<<q<<" "<<s2.length()<<endl;
                        string s3=s2.substr(0,q);
                        string s4=s2.substr(q+1,s2.length());
                        // cout<<s1<<" "<<s2<<" "<<s3<<" "<<s4<<endl;
                        string ye=')'+a;
                        if(stoi(s2)!=id){
                            // cout<<"second "<<s1+"^"+s3<<endl;
                            if(stoi(nbrd2files[s1+"^"+s3].first)>stoi(s4)){
                                nbrd2files[s1+"^"+s3].first=s4;
                            }
                            (nbrd2files[s1+"^"+s3].second)++;
                            if((nbrd2files[s1+"^"+s3].second)==nbrs-1){
                                for(int i=0;i<nbrs;i++){
                                    if(IDs[sendfd[i]]==stoi(s2)){
                                        n_bytes=send(sendfd[i],ye.c_str(), strlen(ye.c_str()), 0);
                                    }
                                }
                            }
                        }
                        else{
                            if(d2files[s1].first>stoi(s4)){
                                int r=s4.find('^');
                                string s5=s4.substr(0,r);
                                string s6=s4.substr(r+1,s4.length());
                                d2files[s1].first=stoi(s5);
                                d2files[s1].second=stoi(s6);
                            }
                            count--;
                        }
                    }
                    for(auto a:no){
                        p=a.find('^');
                        s1=a.substr(0,p);
                        s2=a.substr(p+1,a.length());
                        string ye='('+a;
                        if(stoi(s2)!=id){
                            (nbrd2files[a].second)++;
                            if((nbrd2files[a].second)==nbrs-1){
                                for(int i=0;i<nbrs;i++){
                                    if(IDs[sendfd[i]]==stoi(s2)){
                                        n_bytes=send(sendfd[i],ye.c_str(), strlen(ye.c_str()), 0);
                                    }
                                }
                            }
                        }
                        else{
                            count--;
                        }
                    }
                    for(auto a:l1){
                        l1confirm++;
                    }
                    for(auto a:l2){
                        l2confirm++;
                    }
                    if(count==0){
                        break;
                    }
                }
            }
        }
        if(count==0 || numleft<=0){
            count=nbrs;
            numleft--;
            if(l1confirm==nbrs){
                for(int i=0; i<nbrs;i++){
                    string a="}"+to_string(id);
                    n_bytes=send(sendfd[i], a.c_str(), strlen(a.c_str()), 0);
                }
                l1confirm++;
            }
            if(numleft==0){
                for(int i=0; i<nbrs;i++){
                    string a="{"+to_string(id);
                    n_bytes=send(sendfd[i],a.c_str(), strlen(a.c_str()), 0);
                }
                numleft--;
            }
            if(l2confirm==nbrs && numleft<0 && l1confirm>nbrs){
                break;
            }
        }
    }
    for (auto it=filemap.begin(); it!=filemap.end();it++){
        if((it->second)!=INTMAX){
            cout<<"Found "<<it->first<<" at "<<it->second<<" with MD5 0 at depth 1"<<endl;
        }
    }
    for (auto it=d2files.begin(); it!=d2files.end();it++){
        if(((it->second).first)==INTMAX){
            cout<<"Found "<<it->first<<" at 0 with MD5 0 at depth 0"<<endl;
        }else{
            cout<<"Found "<<it->first<<" at "<<((it->second).first)<<" with MD5 0 at depth 2"<<endl;
        }
    }
    // int numd2=0;
    // for (auto it=d2files.begin(); it!=d2files.end();it++){
    //     if(((it->second).first)==INTMAX){
    //         d2files.erase(it->first);
    //     }else{
    //         numd2++;
    //     }
    // }
}

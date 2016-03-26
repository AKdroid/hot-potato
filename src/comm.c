#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/fcntl.h>
#include<sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<sys/select.h>



char *trim(char *s){
    
    int l;  
    char *x,*y;
    l =strlen(s);
    x = s;
    while(*x==' '||*x=='\t'||*x=='\n'){
        x++;
    }
    y=s+l-1;
    while(*y==' '||*y=='\t'||*y=='\n'){
        y--;
    }
    y++;
    *y=0;
    return x;
}


struct sockaddr_in* initialize_master(int portnum, int* s){
    
    struct sockaddr_in* master_socket=NULL;
    struct hostent *hp=NULL;
    int setflag=1,retval;
    char hostname[64];
    *s = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(*s,SOL_SOCKET,SO_KEEPALIVE,&setflag,1);
    setsockopt(*s,SOL_SOCKET,SO_KEEPALIVE,&setflag,1);
    if(*s < 0){
        perror("Socket:");
        return NULL;
    }

    gethostname(hostname, sizeof hostname);
    hp = gethostbyname(hostname);

    if(hp == NULL){
        perror("gethostbyname:");
        return NULL;
    }

    master_socket = (struct sockaddr_in*) malloc (sizeof(struct sockaddr_in));
    
    if(master_socket == NULL){
        perror("malloc:");
        return NULL;
    }

    master_socket->sin_family = AF_INET; 
    master_socket->sin_port = htons(portnum);

    memcpy(&master_socket->sin_addr,hp->h_addr_list[0],hp->h_length);

    retval = bind(*s, (struct sockaddr*) master_socket, sizeof(*master_socket));
    
    if(retval < 0){
        perror("Bind: ");
        exit(1);
    }

    return master_socket;
}

struct sockaddr_in* connect_to_master(char* hostname, int portnum, int* s){

    struct sockaddr_in* master_socket=NULL;
    struct hostent *hp=NULL;
    int rc;
    int setflag=1;
    *s = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(*s,SOL_SOCKET,SO_KEEPALIVE,&setflag,1);
    if(*s < 0){
        perror("Socket:");
        return NULL;
    }
    
    hp = gethostbyname(hostname);
    
    if(hp == NULL){
        perror("gethostbyname:");
        return NULL;
    }

    master_socket = (struct sockaddr_in*) malloc (sizeof(struct sockaddr_in));

    if(master_socket == NULL){
        perror("malloc:");
        return NULL;
    }

    master_socket->sin_family = AF_INET;
    master_socket->sin_port = htons(portnum);  

    memcpy(&master_socket->sin_addr,hp->h_addr_list[0],hp->h_length);

    rc = connect(*s, (struct sockaddr *)master_socket, sizeof(*master_socket));

    if(rc < 0){
        perror("Connect: ");
        return NULL;
    }

    return master_socket;
}

void register_client(int master,int *id, int* lid, int* rid){
    
    char sendbuffer[1024];
    char *readbuffer = NULL;
    char hostname[64],temp[13];
    char*x,*y;
    int sent,rcvd;    
    char cmd[100];
    char * body= NULL;
    int size;

    gethostname(hostname,sizeof(hostname));
    
    sprintf(sendbuffer, "#REGISTER# %s #END#",hostname);
    
    sent = write_to_socket(master, sendbuffer);

    if(sent < 0){
        perror("Sent:");
        return;
    }
    
    read_from_socket(master,&readbuffer,&rcvd);
    if(rcvd > 0){
        parse_command(readbuffer,cmd,&size,&body);
        body = trim(body);
        if(strcmp(cmd,"SETID")==0){
            *id = atoi(body);
            x = strstr(body,",");
            *lid = atoi(x+1);
            x = strstr(x+1,",");
            *rid = atoi(x+1);
        }
        free(readbuffer);
    }
    if(rcvd < 0){
        perror("read");
    }

    sprintf(sendbuffer,"#ACK# %d #END#",0);
    
    sent = write_to_socket(master,sendbuffer);

    if(sent < 0){
        perror("Sent:");
        return;
    }

}

void allocate_id(int client_socket, int id, int lid, int rid,  char* hostname){
    
    char sendbuffer[1024];
    char *readbuffer = NULL;
    char temp[13];
    int sent,rcvd,res,size;
    char* x;
    char *body = NULL;
    char cmd[100];

    read_from_socket(client_socket, &readbuffer,&rcvd);

    if(rcvd > 0){
        
        parse_command(readbuffer,cmd,&size,&body);
        body = trim(body);
        if(strcmp(cmd,"REGISTER")==0){
            strcpy(hostname,body);   
            //printf("Hostname of client = %s\n",hostname);
        }
        else
            printf("Error: Invalid command\n");
        free(readbuffer);
    }

    if(rcvd < 0){
        perror("read");
    }
    
    sprintf(sendbuffer,"#SETID# %d,%d,%d #END#",id,lid,rid);

    sent = write_to_socket(client_socket,sendbuffer);

    if(sent < 0){
        perror("Sent:");
        return;
    }

    read_from_socket(client_socket,&readbuffer,&rcvd);
    if(rcvd > 0){
        parse_command(readbuffer,cmd,&size,&body);
        if(strcmp(cmd,"ACK")!=0){
            printf("ACK was not received\n");
        }
        free(readbuffer);
    }
    return 1;
}

void read_from_socket(int s, char** rcvd, int* rcvdsize){

    char buf[512];
    char size_in_words[12];
    int size=-1;
    int rd,retval,cnt=0;
    char cmd[20];
    char* fullbuf,*temp;
    char *body;
    int attempts = 150;
    while(attempts > 0){
      rd = recv(s,buf,512,MSG_PEEK);
      if(rd > 0){
        buf[rd]=0;
        //printf("%d %s\n",rd,buf);
        retval=parse_command(buf,&cmd,&size,&body);
        if(retval >= 0){
            break;
        }
      }
      attempts --; 
      sleep(5);      
    }
    if(size == -1){
        *rcvd=NULL;
        *rcvdsize == -1;
        return;
    }
    sprintf(size_in_words,"%d",size);
    size = size + strlen(size_in_words)+20;
    fullbuf = (char*) malloc(size);
    if(fullbuf == NULL){
        perror("malloc:readbuffer ");
        *rcvd = NULL;
        *rcvdsize = -1;
        return;
    }    
    cnt = 0;
    temp = fullbuf;
    while(1){
        rd = recv(s,temp,size,0);
        temp[rd] = 0;
        size = size - rd;
        cnt = cnt +rd;
        if (strcmp(temp+rd-5,"#END#")==0)
            break;
        temp=temp+rd;
    }
    //printf("fullbuf=%s\n",fullbuf);
    *rcvd = fullbuf;
    *rcvdsize=cnt ;
}

int write_to_socket(int s, char* payload){

    char* frame= NULL;
    int size,payload_size,res;     
    char cmd[20];
    char size_w[12];
    char *breakpoint;

    payload_size = strlen(payload);
    sprintf(size_w,"%d",size);


    size = payload_size + strlen(size_w) + 20;
    frame = (char*)malloc(size);

    sscanf(payload,"#%s# %s #END#",cmd);

    breakpoint= strstr(payload, cmd) + strlen(cmd) + 1;

    //printf("cmd: %s breakpoint :%s\n", cmd,breakpoint);

    sprintf(frame, "#%s #%d# %s",cmd,payload_size,breakpoint);

    //printf("frame: %s\n",frame);

    res = send(s,frame,strlen(frame),0);    

    free(frame);

    return res;    

}

int parse_command(char* buf, char* cmd, int *size , char** body){

    char *x,*y;
    char z[12];

    int count = 0,hflag=0, bflag=0,i=0;
    x = buf;
    count=0;
    y=z;
    *body = NULL;
    while(*x!=0){
        if(*x == '#'){
            hflag=1-hflag;
            count = count + 1;
            if(count  == 4 && bflag == 0){
                bflag=1;
                *body = x+2;
                *size = atoi(z);
                *y=0;
            }
                
            if(count == 5){
                *cmd = 0;
                *x=0;
                if(strncmp(x,"#END#",5)==0)
                    return 1;
            }
            ++x;
            ++i;
            continue;
        }
        if(hflag == 1){
            if(count == 1){
                *cmd=*x;
                cmd++;
            }
            if(count == 3){
                *y=*x;
                y++;
            }
            
        }
        ++x;
        ++i;
    }
    *cmd = 0;
    *y=0;
    if(count < 4)
        return -1;

    if(count == 4)
        return 0;

    return 1;

}



void setup_left(int s, int* listening_port){

    char sendbuffer [100];
    char * readbuffer = NULL;
    char* body;
    char cmd[20];
    int sent,rcvd,size;

    *listening_port = -1;    

    sprintf(sendbuffer,"#SETLEFT# %d #END#",0);

    sent = write_to_socket(s,sendbuffer);

    if(sent<0){
        perror("send: ");
        return;
    }
        
    read_from_socket(s,&readbuffer,&rcvd);

    if(rcvd > 0){
        parse_command(readbuffer,cmd,&size,&body);
        if(strcmp(cmd,"ACK")==0){
            *listening_port=atoi(body);
        }
        free(readbuffer);
    }
}

void setup_right(int s, char* hostname, int portnum){

    char sendbuffer [100];
    char * readbuffer = NULL;
    char* body;
    char cmd[20];
    int sent,rcvd,size;


    //printf("hostname = %s portnum = %d\n",hostname,portnum);

    sprintf(sendbuffer,"#SETRIGHT# %s:%d #END#",hostname,portnum);

    sent= write_to_socket(s,sendbuffer);

    if(sent<0){
        perror("send: ");
        return;
    }

    read_from_socket(s,&readbuffer,&rcvd);

    if(rcvd > 0){
        parse_command(readbuffer,cmd,&size,&body);
        if(strcmp(cmd,"ACK")!=0){
            printf("Connection Failed\n");
        }
        free(readbuffer);
    }

}

void connect_to_neighbor(int master, struct sockaddr_in** left, struct sockaddr_in** right, int* left_s, int* right_s){

    char* readbuffer = NULL,cmd[20];
    char sendbuffer[100];
    char * body,*x;
    int rcvd;
    int size,sent;
    int len;
    char hostname[64];
    int rport,retval;
    int flag=-1;
    int sockid;

    struct sockaddr_in *lner;
    struct sockaddr_in ltemp;

    //printf("Waiting for the neighbor details\n");

    read_from_socket(master,&readbuffer,&rcvd);

    if(rcvd > 0){
        parse_command(readbuffer,cmd,&size,&body);
        if(strcmp(cmd,"SETLEFT")==0)
            flag=0;
        else if(strcmp(cmd,"SETRIGHT")==0){
            flag=1;
            body = trim(body);
            x = strstr(body,":");
            //printf("body = %s, diff=%d\n",body,x-body);
            strncpy(hostname,body,x-body);
            hostname[x-body]=0;
            rport =  atoi(x+1);
            //printf("Hostname: %s Port = %d\n",hostname,rport);
        }
        free(readbuffer);
    }

    //printf("Received cmd=%s\n",cmd);

    if(flag == 0){
        lner = initialize_master(0,&sockid);
        
        if(lner == NULL)
            return;

        listen(sockid,2);
        len = sizeof(struct sockaddr);
        retval = getsockname(sockid, (struct sockaddr*) &ltemp, &len);
        if(retval < 0){
            perror("GetSockName: ");
            return;
        }
        rport = ntohs(ltemp.sin_port);
        
        sprintf(sendbuffer,"#ACK# %d #END#",rport);
        
        sent = write_to_socket(master,sendbuffer);
        if(sent < 0){
            perror("Sent: ");
        } 

        len = sizeof(struct sockaddr);
        *left_s = accept(sockid,(struct sockaddr*) *left, &len);
       
        //close(sockid);
        //free(lner);
 
        test_connection(*left_s, 0);

    }
    else if(flag == 1){
         
        *right = connect_to_master(hostname,rport,right_s);

        test_connection(*right_s, 1);

        if(*right != NULL)
        sprintf(sendbuffer,"#ACK# %d #END#",0);
   
        sent = write_to_socket(master,sendbuffer);
        if(sent < 0){
            perror("Sent: ");
        }

    }
}

int test_connection(int s, int flag){

    char sendbuffer[100],cmd[20];
    char *readbuffer = NULL;
    char *body;
    int sent,rcvd,size;

    if(flag == 0){
        read_from_socket(s,&readbuffer,&rcvd);
        if(rcvd > 0){
            parse_command(readbuffer,cmd,&size,&body);
            if(strcmp(cmd,"PING")==0){
                strcpy(sendbuffer,"#PONG# 0 #END#");
                sent = write_to_socket(s, sendbuffer);
                if(sent<0){
                    perror("Sent: ");
                }
            }else{
                printf("Connection Failed\n");
                return -1;
            }
            free(readbuffer);    
        }
        
    }else if(flag == 1){
        strcpy(sendbuffer,"#PING# 0 #END#");
        sent = write_to_socket(s, sendbuffer);
        if(sent<0){
            perror("Sent: ");
            return;
        }
        read_from_socket(s,&readbuffer,&rcvd);
        if(rcvd > 0){
            parse_command(readbuffer,cmd,&size,&body);
            if(strcmp(cmd,"PONG")!=0){
                printf("Connection Failed\n");
                return -1;
            }
            free(readbuffer);
        }
    }
    return 0;
}

void initiate_game(int player_sock, int hops){

    char sendbuffer[100];
    int sent;    

    sprintf(sendbuffer,"#START# %d #END#",hops);
    
    sent= write_to_socket(player_sock,sendbuffer);
    
    if(sent < 0){
        perror("Send: ");
    }

}

int select_readable_socket(int * sockets, int length, int timeout_sec){

    fd_set read_set;
    int i;
    int retval;
    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;

    FD_ZERO(&read_set);
    for(i=0;i<length;i++){
        FD_SET(sockets[i],&read_set);
    }
    
    retval = select(FD_SETSIZE,&read_set,NULL,NULL,&timeout);    

    if(retval>0){
        for(i=0;i<length;i++)
            if(FD_ISSET(sockets[i],&read_set)){
                return sockets[i];
            }

    }
    return -1;
}


void get_potato(int sock, int* hops, char ** content, char **mainbody){

    char * readbuffer=NULL,*body,*x,cmd[20];
    int size, rcvd=0;
    
    read_from_socket(sock, &readbuffer, &rcvd);
    if(rcvd > 0){
        parse_command(readbuffer,cmd,&size,&body);
        body=trim(body);
        if(strcmp(cmd,"CLOSE")==0){
            *content = readbuffer;
            *hops=-1;
            return;
        }
        if(strcmp(cmd,"PASS")==0){
            *hops = atoi(body);
            *content = readbuffer;
            *mainbody = strstr(body,":")+1;
        }
        else if(strcmp(cmd,"START")==0){
            *hops = atoi(body);
            *content = readbuffer;
            *mainbody = NULL;
        }
    }
}

void pass_potato(int sock, int id, int hops, int comma, char *body){

    char *sendbuffer = NULL;
    char size_s[12],hops_s[12];
    int size,sent;
    if(comma == 1){
        sprintf(size_s,",%d",id);
    } else {
        sprintf(size_s,"%d",id);
    }
    sprintf(hops_s,"%d:",hops-1);
    size = strlen(body) +  strlen(size_s) + strlen(hops_s) + 7 + 7;
    sendbuffer = (char*) malloc(size);
    
    sprintf(sendbuffer,"#PASS# %s%s%s #END#",hops_s,body,size_s);
    
    sent = write_to_socket(sock,sendbuffer);

    if(sent < 0){
        perror("sent: ");
    }

    free(sendbuffer);

}

void close_players(int sock){

    char sendbuffer[100];
    int sent;    

    strcpy(sendbuffer,"#CLOSE# 0 #END#");

    sent = write_to_socket(sock,sendbuffer);
    if(sent < 0)
        perror("Sent: ");

}

void print_final_trace(int sock){

    char *readbuffer=NULL,cmd[20],*body,*x;
    int size,rcvd,hops;

    read_from_socket(sock,&readbuffer,&rcvd);
    if(rcvd > 0){
        parse_command(readbuffer,cmd,&size,&body);
        if(strcmp(cmd,"PASS")==0){
            body = trim(body);
            hops = atoi(body);
            if(hops == 0){
                x=strstr(body,":");
                printf("Trace of potato\n");
                printf("%s\n",x+1);
            }
        }
        free(readbuffer);
    }    
}


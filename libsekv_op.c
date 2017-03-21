#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "encrypt.h"
#include "hash_op.h"

int sekv_connect_server()
{
    int client_sockfd;
    struct sockaddr_in remote_addr; //服务器端网络地址结构体
    memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零  
    remote_addr.sin_family=AF_INET; //设置为IP通信  
//    remote_addr.sin_addr.s_addr=inet_addr("10.214.16.146");//服务器IP地址  
    remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");//服务器IP地址  
    remote_addr.sin_port=htons(11211); //服务器端口号  

    /*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
    if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket");
        return 1;
    }

    /*将套接字绑定到服务器的网络地址上*/
    if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
    {
        perror("connect");
        return 1;
    }
    return client_sockfd;
}

int sekv_set(int sockfd, char *key, int flags, int exptime, int bytes, char *value)
{
    char buf[BUFSIZ], comm[BUFSIZ], bufv[BUFSIZ];
    // <command name> <key> <flags> <exptime> <bytes> [noreply]\r\n
    char *command="set ";
    strcpy(comm, command);
    // encrypt <key>
    unsigned char *p_enc, *p_mac;
    int len_dst;
    p_enc = (unsigned char *)malloc(sizeof(unsigned char)*strlen(key));
    p_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);
    my_aes_gcm_encrypt(key,strlen(key),p_enc,&len_dst,p_mac);
    printf("strlen(p_mac):%d\n",strlen(p_mac));
    // the return MAC length may vary, we use the first 16 bytes
    strncat(comm,p_mac,16);
    strcat(comm,p_enc);
    // the value length bytes+16, 16 is the length of MAC
    sprintf(buf," %d %d %d",flags,exptime,bytes+16);
    strcat(comm,buf);
    strcat(comm,"\r\n");    
    printf("command:\n");
    BIO_dump_fp(stdout,comm,strlen(comm));
    int len;
    // send set command
    len=send(sockfd,comm,strlen(comm),0);
    //store key mac
    char *md_value;
    int md_len;
    md_value = (unsigned char *)malloc(sizeof(unsigned char)*20);
    my_sha1(comm,md_value,&md_len);
 
    free(p_enc);
    free(p_mac);
    // encrypt <MACmeta, vn, MACv,value>
    p_enc = (unsigned char *)malloc(sizeof(unsigned char)*strlen(value));
    p_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);
    my_aes_gcm_encrypt(value,strlen(value),p_enc,&len_dst,p_mac);
    strncpy(bufv,p_mac,16);
    strcat(bufv,p_enc);
    strcat(bufv,"\r\n");
//    printf("value:\n");
//    BIO_dump_fp(stdout,bufv,strlen(bufv));
//    printf("strlen(bufv):%d\n",strlen(bufv));
    // send encrypted <value>
    len=send(sockfd,bufv,strlen(bufv),0); 
    len=recv(sockfd,buf,BUFSIZ,0);
    printf("%s\n",buf);   
    return 0;
}

int sekv_get(int sockfd, char *key, int flags, int exptime, int bytes, char *value)
{
    char buf[BUFSIZ], comm[BUFSIZ];
    char *command="get ";
    strcpy(comm,command);
    unsigned char *p_enc, *p_mac;
    int len_dst;
    p_enc = (unsigned char *)malloc(sizeof(unsigned char)*strlen(key));
    p_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);
    my_aes_gcm_encrypt(key,strlen(key),p_enc,&len_dst,p_mac);
    strncat(comm,p_mac,16);
    strcat(comm,p_enc);
    strcat(comm,"\r\n");
    int len;
    len=send(sockfd,comm,strlen(comm),0);
    printf("command:\n");
    BIO_dump_fp(stdout,comm,strlen(comm));
    int receive_sum=0,index=0;
    receive_sum = 1024;
    len=0;
//    while(receive_sum > 0){
//       printf("receive_sum:%d\n",receive_sum);
       len=recv(sockfd,buf+index,receive_sum,0);
       printf("len:%d\n",len);
//       if(len <= 0 ) break;
       receive_sum -= len;
       index += len;
//    }
    printf("Received Value:\n");
    BIO_dump_fp(stdout,buf,strlen(buf));
    
    int i,line,j;
    char ev[BUFSIZ];
    char *p_dec,*p_dst;
    line=0;j=0;
    for(i=0;i<strlen(buf);i++){
       if(line==1){
         ev[j]=buf[i];j++;
       }
       if(buf[i]=='\n'){
         line++;
       }
    }
    p_dec = (unsigned char *)malloc(sizeof(unsigned char)*1024);
    p_dst = (unsigned char *)malloc(sizeof(unsigned char)*1024);
    strncpy(p_mac,ev,16);
    strncpy(p_dec,ev+16,strlen(ev)-16-2);
    printf("p_mac:\n");
    BIO_dump_fp(stdout,p_mac,16);
    BIO_dump_fp(stdout,p_dec,strlen(ev)-16-2); 
    
    my_aes_gcm_decrypt(p_dec,strlen(ev)-18,p_dst,&len_dst,p_mac);
    printf("plain value:\n");
    printf("%s\n",p_dst);
    return 0;
}

int main()
{
   char *key="1234567891011121";
   char *value="hell11111111111111111";
   int re=0;
   int sockfd;
   sockfd = sekv_connect_server();
   re = sekv_set(sockfd,key,0,0,strlen(value),value);
   re = sekv_get(sockfd,key,0,0,0,NULL);
} 

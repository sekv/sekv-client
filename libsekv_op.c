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
    char buf[BUFSIZ], comm[BUFSIZ], bufv[BUFSIZ],sb[BUFSIZ];
    // <command name> <key> <flags> <exptime> <bytes> [noreply]\r\n
    char *command="set ";
    strcpy(comm, command);
    // encrypt <key>
    unsigned char *p_enc, *p_mac;
    int len_dst;
    p_enc = (unsigned char *)malloc(sizeof(unsigned char)*strlen(key));
    p_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);
    my_aes_gcm_encrypt(key,strlen(key),p_enc,&len_dst,p_mac);
    printf("strlen(p_enc):%d\n",strlen(p_enc));
    printf("strlen(p_mac):%d\n",strlen(p_mac));
    // the return MAC length may vary, we use the first 16 bytes
    strncat(comm,p_mac,16);
    strcat(comm,p_enc);
    // the value length bytes+16, 16 is the length of MAC, 20 bytes of meta sha1, 4 bytes of version number
    sprintf(buf," %d %d %d",flags,exptime,bytes+16+20+4);
    strcat(comm,buf);
    strcat(comm,"\r\n");    
    printf("command:\n");
    BIO_dump_fp(stdout,comm,strlen(comm));
    int len;
    // send set command
    len=send(sockfd,comm,strlen(comm),0);
    
    // version tracking table operations
    item *it_new, *it;
    uint32_t hv;
    it_new = (item *)malloc(sizeof(item));
    it_new->key = (char *)malloc(sizeof(char)*16);
    it_new->nkey = 16;
    it_new->vn = 0;
    strncpy(it_new->key,p_mac,16);
    hv = MurmurHash3_x86_32(it_new->key, it_new->nkey);
    it = assoc_find(it_new->key, it_new->nkey, hv);
    if(it){
        assoc_delete(it_new->key,it_new->nkey, hv);
        printf("set command: find the same key!\n");   
    }else{
        printf("set command: not find the same key!\n");
    }
    assoc_insert(it_new, hv);

    free(p_enc);
    free(p_mac);
    // encrypt <MACmeta,vn,value>
    p_enc = (unsigned char *)malloc(sizeof(unsigned char)*(strlen(value)+20+4));
    p_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);
    // MACmeta is the sha1 message digest
    unsigned char *md_value;
    int md_len;
    md_value = (unsigned char *)malloc(sizeof(unsigned char)*20);
    sprintf(buf,"%s %d %d",key,flags,exptime);
    printf("meta:%s\n",buf);
    my_sha1(buf,md_value,&md_len);
    memset(bufv,0,BUFSIZ);
    strncpy(bufv,md_value,20);
    // 4 bytes long version number, initial value 0
    sprintf(buf,"%4d",0);
    strncat(bufv,buf,4);
    strcat(bufv,value);
    printf("bufv:%s\n",bufv);
    printf("strlen(bufv):%d\n",strlen(bufv));
    my_aes_gcm_encrypt(bufv,strlen(bufv),p_enc,&len_dst,p_mac);
    
    memset(sb,0,BUFSIZ);
    strncpy(sb,p_mac,16);
    strcat(sb,p_enc);
    strcat(sb,"\r\n");
    printf("value:\n");
    BIO_dump_fp(stdout,sb,strlen(sb));
    printf("strlen(bufv):%d\n",strlen(sb));
    // send encrypted <value>
    len=send(sockfd,sb,strlen(sb),0); 
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
    printf("strlen(key):%d\n",strlen(key));
    p_enc = (unsigned char *)malloc(sizeof(unsigned char)*strlen(key));
    p_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);
    my_aes_gcm_encrypt(key,strlen(key),p_enc,&len_dst,p_mac);
    printf("strlen(p_enc):%d\n",strlen(p_enc));
    strncat(comm,p_mac,16);
    strncat(comm,p_enc,strlen(key));
    strcat(comm,"\r\n");
    int len;
    len=send(sockfd,comm,strlen(comm),0);
   
    //version tracking table operations 
    item *it;
    uint32_t hv;
    hv = MurmurHash3_x86_32(p_mac,16);
    it = assoc_find(p_mac,16,hv);

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
    printf("strlen(buf):%d\n",strlen(buf));
    
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
    unsigned char *shameta,*vn;
    p_dec = (unsigned char *)malloc(sizeof(unsigned char)*1024);
    p_dst = (unsigned char *)malloc(sizeof(unsigned char)*1024);
    shameta = (unsigned char *)malloc(sizeof(unsigned char)*20);
    vn = (unsigned char *)malloc(sizeof(unsigned char)*4);
    strncpy(p_mac,ev,16);
//    strncpy(shameta,ev+16,20);
//    strncpy(vn,ev+16+20,4);
    strncpy(p_dec,ev+16,strlen(ev)-16-2);
    printf("p_mac:\n");
    BIO_dump_fp(stdout,p_mac,16);
    printf("p_dec:\n");
    BIO_dump_fp(stdout,p_dec,strlen(ev)-16-2); 
    printf("strlen(ev):%d\n",strlen(ev)); 
    my_aes_gcm_decrypt(p_dec,strlen(ev)-18,p_dst,&len_dst,p_mac);
    strncpy(shameta,p_dst,20);
    strncpy(vn,p_dst+20,4);
    printf("vn:%d\n",atoi(vn));
    if(it){
      if(it->vn == atoi(vn)){
         printf("Version number checking success!\n");
      }
      else{
         printf("Version number checking failed!\n"); 
      }
    }
    else{
      printf("Not find in version tracking table!\n");
    }
    printf("plain value:\n");
    printf("%s\n",p_dst+24);
    return 0;
}

int sekv_append(int sockfd, char *key, int flags, int exptime, int bytes, char *value)
{
   
}

int main()
{
   char *key="1234key";
   char *value="hello world! My first runnable SeKV get set operations";
   int re=0;
   int sockfd;
   assoc_init(16);
   sockfd = sekv_connect_server();
   re = sekv_set(sockfd,key,0,0,strlen(value),value);
   re = sekv_get(sockfd,key,0,0,0,NULL);
   re = sekv_set(sockfd,key,0,0,strlen(value),value);
   re = sekv_get(sockfd,key,0,0,0,NULL);
} 

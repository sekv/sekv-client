#include <stdio.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <math.h>
int    time_substract(struct timeval *result, struct timeval *begin,struct timeval *end)
{

    if(begin->tv_sec > end->tv_sec)
      return -1;

    if((begin->tv_sec == end->tv_sec) && (begin->tv_usec > end->tv_usec))
      return -2;

    result->tv_sec    = (end->tv_sec - begin->tv_sec);
    result->tv_usec    = (end->tv_usec - begin->tv_usec);
    if(result->tv_usec < 0)
    {
        result->tv_sec--;
        result->tv_usec += 1000000;
    }
    return 0;
}

  
int main(int argc, char *argv[])  
{  
    int client_sockfd;  
    int len,value_len,key,i;  
    struct sockaddr_in remote_addr; //服务器端网络地址结构体  
    char buf[1100000], comm[BUFSIZ];  //数据传送的缓冲区  
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
    printf("connected to server\n");  
    char *command="get ";
    char num[255];
//    printf("1");
    strcpy(comm,command);
//    printf("2");
//    scanf("%d",&key);
//    sprintf(num,"%d",key);
    strcat(comm,argv[1]);
//    strcat(comm," 0 0 ");
//    scanf("%d",&value_len);
//    printf("3");
//    sprintf(num,"%d",value_len);
//    printf("4");
//    strcat(comm,argv[2]);
//    printf("5");
    strcat(comm,"\r\n");
//    printf("6");
    printf("%s",comm);
//    printf("strlen(comm):%d\n", strlen(comm));
    
    char *value;
    int receive_sum=0,index=0;
//    value = (char *)malloc(sizeof(char)*(atoi(argv[2])+2));
//    memset(value,'a', atoi(argv[2]));
//    strcat(value,"\r\n");

struct timeval t_start,t_end,t_diff;
    memset(&t_start,0,sizeof(struct timeval));
    memset(&t_end,0,sizeof(struct timeval));
    memset(&t_diff,0,sizeof(struct timeval));

gettimeofday(&t_start, NULL);

    len=send(client_sockfd,comm,strlen(comm),0);
    
//    memcpy(buf,value,strlen(value));
//    len=send(client_sockfd,value,atoi(argv[2])+2,0);
//    len=recv(client_sockfd,buf,BUFSIZ,0);//接收服务器端信息  
   receive_sum = 1024; 
   printf("atoi(argv[1]):%d\n",atoi(argv[1]));
   for(i=1;i<atoi(argv[1]);i++){
    receive_sum = receive_sum*2;}
    receive_sum -= 500;
    printf("Receive:%d\n",receive_sum);
    
    while(receive_sum > 0 ){
        len=recv(client_sockfd,buf+index,receive_sum,0);
         if(len <= 0) break;
        receive_sum -= len;
        index += len;
    }
gettimeofday(&t_end, NULL);

time_substract(&t_diff,&t_start,&t_end);
printf("time cost is: %u s, %u us.\n", t_diff.tv_sec, t_diff.tv_usec);

//    buf[len]='\0';  
//    printf("%s",buf); //打印服务器端信息  
      
//    /*循环的发送接收信息并打印接收信息--recv返回接收到的字节数，send返回发送的字节数*/  
////    while(1)  
//    {  
//        int i;
//        printf("Enter string to send:\n");  
////        scanf("%[^\n]",buf);  
//        gets(buf);
//        if(!strcmp(buf,"quit"))  
////            break;
//
//        printf("%s",buf);
//        i = strlen(buf);
//        printf("i:%d\n",i);
//        buf[i]='\r';
//        buf[i+1]='\n';      
//        printf("strlen(buf):%d\n",strlen(buf));
//        printf("%s",buf); 
//        len=send(client_sockfd,buf,strlen(buf),0);
//        gets(buf1);
//        printf("%s",buf1);
//        i = strlen(buf1);
//        printf("i:%d\n",i);
////        if(!strcmp(buf1,"quit"))  
////            break;
//        buf1[i]='\r';
//        buf1[i+1]='\n';  
////        buf[i+2]='\0';
//        printf("strlen(buf1):%d\n",strlen(buf1));
//        len=send(client_sockfd,buf1,strlen(buf1),0);
//  
//        len=recv(client_sockfd,buf1,BUFSIZ,0);  
//        buf[len]='\0';  
//        printf("received:%s\n",buf1);  
//    }  
    close(client_sockfd);//关闭套接字  
        return 0;  
}

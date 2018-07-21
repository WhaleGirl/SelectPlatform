//web的资源一般会保存在web的根目录下
#include<stdio.h>
#include<pthread.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<assert.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<string.h>
#include<signal.h>
#define MAX 1024
#define HOME_PAGE "index.html"
#define PAGE_404 "wwwroot/404.html"
static void Usage(const char* proc)
{
	printf("usage: %s port\n",proc);
}

int	get_line(int sock,char line[],int num)
{
	assert(line);
	assert(num>0);
	int i = 0;
	char c = 'A';
	while(c!='\n'&& i<num-1)
	{
			ssize_t s = recv(sock,&c,1,0);//将读到的字符读到c里
			if(s>0){
				if(c=='\r'){
					recv(sock,&c,1,MSG_PEEK);//窥探下一个字符是否是'\n'
					if(c=='\n'){
						//如果窥探到下一个\n，那么就读走它
						recv(sock,&c,1,0);
					}
					else{
						c='\n';
					}
				}
				//else
				line[i++] = c;
			}
	}
	line[i] = '\0';
	return i;
}

void clear_header(int sock)
{
	char line[MAX];
	do{
		get_line(sock,line,sizeof(line));
		printf("%s\n",line);
	}while(strcmp(line,"\n"));
}
void show_404(int sock)
{
	char line[1024];
	sprintf(line,"HTTP/1.0 404 Not Found\r\n");
	send(sock,line,strlen(line),0);
	sprintf(line,"Content-Type:text/html;charset=ISO-8859-1\r\n");
	send(sock,line,strlen(line),0);
	sprintf(line,"\r\n");
	send(sock,line,strlen(line),0);

	struct stat st;
	stat(PAGE_404,&st);
	int fd = open(PAGE_404,O_RDONLY);
	sendfile(sock,fd,NULL,st.st_size);

	close(fd);
}

void echo_error(int sock,int code)
{
	switch(code){
		case 400:
			printf("error 404\n");
			break;
		case 403:
			break;
		case 404:
			show_404(sock);
			break;
		case 500:
			break;
		case 503:
			break;
		default:
			break;
	}
}

int echo_www(int sock,char* path,int size)//响应资源，往客户端响应
{
//构建响应行，响应报头以及空行，再返回响应
	int fd = open(path,O_RDONLY);
	if(fd<0){
		return 404;
	}
	clear_header(sock);//这个函数一直读一直读，读到空行为止
	//构建响应行
	char line[MAX];
	sprintf(line,"HTTP/1.0 200 OK\r\n");
	//TCP面向字节流，所以上面的line一行可以发送了，但是不是直接发送给对方应用层，而是放在接收缓冲区
	send(sock,line,strlen(line),0);
	

	//构建响应报头
	sprintf(line,"Content-Type:text/html;charset=ISO-8859-1\r\n");
	send(sock,line,strlen(line),0);
	//构建空行
	sprintf(line,"\r\n");
	send(sock,line,strlen(line),0);
	//该接口是直接从内核到内核，不用经过用户，效率高
	//sock fd 不需要设置偏移量
	
	sendfile(sock,fd,NULL,size);
	close(fd);
	return 200;
}

int exe_cgi(int sock,char* method,char* path,char* query_string)
{
	char line[MAX];
	char method_env[MAX/10];
	char query_string_env[MAX];
	char content_length_env[MAX/10];
	int content_length = -1;

	if(strcasecmp(method,"GET")==0)
	{
		clear_header(sock);
	}else {
		//POST
		do{
			get_line(sock,line,sizeof(line));
			if(strncmp(line,"Content-Length: ",16)==0){
				content_length = atoi(line+16);//从第十六个开始转换
			}
		}while(strcmp(line,"\n"));
		if(content_length == -1){
			return 400;
		}
	}

	//站在子进程的角度
	int input[2];//子 读
	int output[2];//子 写
	pipe(input);
	pipe(output);

	//执行模式必须是cgi，path已经知道，请求已经处理完成
	//执行path
	pid_t id = fork();
	if(id<0){
		return 503;
	}else if(id==0){//child
		close(sock);
		close(input[1]);
		close(output[0]);
		//环境变量不会因为程序替换而被替换
		//构建环境变量
		sprintf(method_env,"METHOD=%s",method);
		putenv(method_env);
		if(strcasecmp(method,"GET")==0){
			sprintf(query_string_env,"QUERY_STRING=%s",query_string);
			putenv(query_string_env);
		}else{
			sprintf(content_length_env,"CONTENT_LENGTH=%d",content_length);
			putenv(content_length_env);
		}
		//标准输入重定向到input,程序替换不会替换文件描述符
		dup2(input[0],0);
		dup2(output[1],1);

		//exec*
		//第一个参数表示要执行谁，第二个参数表示要怎么执行
		execl(path,path,NULL);
		exit(1);
	}else{//线程在wait，不会影响其他线程
		close(input[0]);
		close(output[1]);
		char c;
		if(strcasecmp(method,"POST")==0){
			int i  = 0;
			for(;i<content_length;i++)
			{
				recv(sock,&c,1,0);
				write(input[1],&c,1);
			}
		}
		//
		sprintf(line,"HTTP/1.0 200 OK\r\n");
		send(sock,line,strlen(line),0);
		sprintf(line,"Content-Type:text/html;charset=ISO-8859-1\r\n");
		send(sock,line,strlen(line),0);
		sprintf(line,"\r\n");
		send(sock,line,strlen(line),0);

		while(read(output[0],&c,1)>0){
			send(sock,&c,1,0);
		}
		waitpid(id,NULL,0);
	}
	return 200;
}
void* handler_request(void* arg)
{
	int sock = (int) arg;
	char line[MAX];
	char method[MAX/10];
	char url[MAX];
	char path[MAX];
	int i = 0;
	int j = 0;
	int status_code = 200;
	int cgi = 0;
	char* query_string = NULL;
	//读一行读到line这个数组里，以\r,\r\n,\n为结尾，但是统一将这个三个换行符转换为\n
	get_line(sock,line,sizeof(line));
	//printf("get_line:%s\n",line);
	while(i < sizeof(method)-1 && j<sizeof(line) && !isspace(line[j])){
		method[i] = line[j];
		i++;
		j++;
	}
	method[i] = '\0';
	if(strcasecmp(method,"GET")==0){

	}else if(strcasecmp(method,"POST")==0){//POST方法要使用cgi技术
		  cgi = 1;
	}else{
		//方法只能是get和post，如果是其他的方法说明是错误的
		clear_header(sock);
		status_code = 400;
		goto end;
	}
	i=0;
	//GET /a/b/c?name=zhangsan&sex=man HTTP/1.0
	while( j<sizeof(line) && isspace(line[j])){
		j++;
	}
	//走到这里j指向的不是空格
	while(i<sizeof(url)-1 && j<sizeof(line)&& !isspace(line[j])){
		url[i] = line[j];
		i++,j++;
	}
	url[i] = '\0';
	//条件编译
#ifdef DEBUG
	printf("method: %s\n url:%s\n",method,url);
	printf("debug line:%s\n",line);
#endif

	//走到这里知道方法，知道url，是否使用cgi
	//GET带参，参数在问号的右侧,把参数提取到query_string中
	if(strcasecmp(method,"GET")==0){
		query_string = url;
		while(*query_string){
			if(*query_string=='?'){
				*query_string = '\0';
				query_string++;
				cgi = 1;
				break;
			}
			query_string++;
		}
	}
	printf("query_string:%s\n",query_string);
	//method GET/POST   url已经被分离，GET方法参数已经放在query_string中，并且为cgi模式
	//将资源放在web下的根目录
	//将完整路径存放在path里，path里就会有两种存放方式----wwwroot/----wwwroot/a/b/c.html-----
	sprintf(path,"wwwroot%s",url);
	if(path[strlen(path)-1]=='/'){
		strcat(path,HOME_PAGE);
	}

	//printf("path:%s\n",path);
	//path不会以‘\0’结尾

	//stat打开一个文件，若成功返回1，失败返回-1
	struct stat st;
	int x = stat(path,&st);
	if(x<0){
		clear_header(sock);
		status_code = 404;
		goto end;
	}else{
		if(S_ISDIR(st.st_mode)){
			strcat(path,HOME_PAGE);
		}else if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)){
			//判断文件是否有可执行权限，如果有，就以cgi方式运行
			cgi = 1;
		}else{
			//资源存在，资源不是目录也没有可执行权限
			//普通文件DONOTHING
		}

		if(cgi){
			status_code = exe_cgi(sock,method,path,query_string);
		}else{
		//	printf("path echo_www: %s\n",path);
			status_code = echo_www(sock,path,st.st_size);//响应资源，往客户端响应
		}
	}

end:
	if(status_code!=200){
		echo_error(sock,status_code);
	}
	//printf("%s",line);
	close(sock);

}
int startup(int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		exit(2);
	}
	int opt = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(port);

	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
	{
		perror("bind");
		exit(3);
	}
	//设置监听
	if(listen(sock,5)<0)
	{
		perror("listen");
		exit(4);
	}
	return sock;
}

int main(int argc ,char* argv[])
{
	if(argc!=2)
	{
		Usage(argv[0]);
		return 1;
	}
	//创建套接字
	int listen_sock = startup(atoi(argv[1]));
		signal(SIGPIPE,SIG_IGN);
	//让客户端进行连接
	for(; ;)
	{
		struct sockaddr_in client;
		socklen_t len = sizeof(client);
		int sock = accept(listen_sock,(struct sockaddr*)&client,&len);
		if(sock<0)
		{
			perror("accept");
			continue;
		}
		//获得新链接
		//
		printf("Get a New Link\n");
		pthread_t tid;
		pthread_create(&tid,NULL,handler_request,(void*) sock);
		pthread_detach(tid);
	}
	return 0;
}

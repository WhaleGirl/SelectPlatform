#include"comm.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main()
{
	//从浏览器拿参数
	char method[64];
	char buf[1024];
	int content_length = -1;
	if(getenv("METHOD")){
		strcpy(method,getenv("METHOD"));
		if(strcasecmp(method,"GET")==0){
			strcpy(buf,getenv("QUERY_STRING"));
		}else {
			content_length = atoi(getenv("CONTENT_LENGTH"));
			int i = 0;
			for(;i<content_length;i++){
				read(0,buf+i,1);
			}
			buf[i] = '\0';
		}
	}

    //std::string name = "lisi";
	//std::string school = "xjd";
	//std::string hobby = "sleep";
	std::cout<< buf<<std::endl;	
	MYSQL* myfd = mysql_connect();
	mysql_select(myfd,buf);//全部取出
	myclose(myfd);

	 return 0;
}

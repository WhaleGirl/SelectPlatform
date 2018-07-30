#include"comm.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
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
	std::cout<<buf<<std::endl;

//	strtok(buf,"=&");
//	std::string name = strtok(NULL,"=&");
//	strtok(NULL,"=&");
//	std::string school = strtok(NULL,"=&");
//	strtok(NULL,"=&");
//	std::string hobby = strtok(NULL,"=&");

    //std::string name = "zhangsan";
	//std::string sex= "man";
	//std::string age= "29";
	//std::string descText= "kjdfhjksfjs";
	//std::string picPath= "../wwwroot/imag/sunset.jpg";
	strtok(buf,"=&");
	std::string name = strtok(NULL,"=&");
	strtok(NULL,"=&");
	std::string IDnum = strtok(NULL,"=&");
	strtok(NULL,"=&");
	std::string sex = strtok(NULL,"=&");
	strtok(NULL,"=&");
	std::string age = strtok(NULL,"=&");
	strtok(NULL,"=&");
	std::string descText = strtok(NULL,"=&");
	strtok(NULL,"=&");
	std::string picPath = strtok(NULL,"=&");


	MYSQL* myfd = mysql_connect();
	mysql_insert(myfd,name,IDnum,sex,age,descText,picPath);
	myclose(myfd);

	 return 0;
}

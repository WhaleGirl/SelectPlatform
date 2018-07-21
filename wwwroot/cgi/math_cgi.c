#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void math_begin(char* buf)
{
	//first=100&second=200
	//解析参数取出想要的数字

	int x,y;
	sscanf(buf,"first=%d&second=%d",&x,&y);

//	printf("%d+%d=%d\n",x,y,x+y);

	printf("<html>\n");
		printf("<h3>\n");
	
		printf("<br>");
	printf("%d+%d=%d\n",x,y,x+y);
	printf("<br>\n");
	printf("%d-%d=%d\n",x,y,x-y);
	printf("<br>\n");
	printf("%d*%d=%d\n",x,y,x*y);
	printf("<br>\n");
	if(y==0){
		printf("%d/%d = error\n",x,y);
	}else{
		printf("%d/%d = %d<br>",x,y,x/y);
	}
	printf("</h3>\n");
	printf("</html>\n");
	
}
int main()
{
	//拿到浏览器参数，拿环境变量
	//先拿方法
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

	//走到这里说明方法已经完成
	
	//printf("cgi_arg:%s\n",buf);
	math_begin(buf);
	return 0;
}

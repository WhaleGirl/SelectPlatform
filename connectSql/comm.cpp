#include"comm.h"

void test()
{
	std::cout<<"client version:"<<mysql_get_client_info()<<std::endl;
}
MYSQL*	 mysql_connect()
{
	MYSQL* myfd = mysql_init(NULL);
	if(mysql_real_connect(myfd,"127.0.0.1","root","456789","selectInfo",3306,NULL,0)==NULL){
		std::cout<<"connect fail"<<std::endl;
	}
	else
	{
		std::cout<<"connect success"<<std::endl;
	}
	return myfd;
}
int mysql_insert(MYSQL* myfd,std::string &name,std::string& IDnum,std::string& sex,\
			std::string& age,std::string &descText,std::string &picPath)
{
	std::string sql = "insert into CrimePeople (name,IDnum,sex,age,descText,picPath) values (\"";
	sql += name;
	sql += "\",\"";
	sql += IDnum;
	sql += "\",\"";
	sql += sex;
	sql += "\",\"";
	sql += age;
	sql += "\",\"";
	sql += descText;
	sql += "\",\"";
	sql += picPath;
	sql += "\")";

	std::cout<<sql<<std::endl;
	return mysql_query(myfd,sql.c_str());	
}
//对数据进行提取
//
void mysql_select(MYSQL* myfd,char* str)
{
	std::cout<<str<<std::endl;
	std::string sql = "select * from CrimePeople where IDnum=";
	sql+=(str+6);
	mysql_query(myfd,sql.c_str());
	MYSQL_RES* result = mysql_store_result(myfd);
	int lines = mysql_num_rows(result);
	int cols = mysql_num_fields(result);
	MYSQL_FIELD* myfield = mysql_fetch_field(result);

	int i = 0;
	std::cout<<"<table border=\"1\">"<<std::endl;
	for(i=0;i<cols;i++)
	{
		std::cout<<"<th>"<<myfield[i].name<<"</th>";
	}

	int j =0;
	MYSQL_ROW row;
	for(i=0;i<lines;i++)
	{
		std::cout<<"<tr>"<<std::endl;
		row = mysql_fetch_row(result);
		for(j =0 ;j<cols;j++)
		{
			std::cout<<"<td>"<<row[j]<<"</td>";
		}
		std::cout<<std::endl;
		std::cout<<"</tr>"<<std::endl;
	}
	std::cout<<"</table>"<<std::endl;

	//free(result);
}
void myclose(MYSQL* myfd)
{
	mysql_close(myfd);
}

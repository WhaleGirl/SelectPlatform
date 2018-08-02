#pragma once
#include<iostream>
#include<mysql.h>
#include<unistd.h>
#include<string.h>
void test();
MYSQL* mysql_connect();
int mysql_insert(MYSQL* myfd,std::string &name,std::string& Idnum,std::string& sex,\
			std::string& age,std::string &descText,std::string &picPath);
void myclose(MYSQL* mysql);
void mysql_select(MYSQL* myfd,char* str);

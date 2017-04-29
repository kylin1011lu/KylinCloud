#include <stdio.h>

//函数指针作为返回值

//声明一个函数getbig
//参数是2个int ,返回值是int
int getbig(int i,int j);


typedef int (*f)(int,int);

//声明一个函数fun
//参数是1个int,返回值是1个函数指针
//该函数指针参数2个int,返回值是1个int
//int (*fun(int a))(int,int);
f fun(int a);


int main(int argc,char*argv)
{
	int max = 0;
	//int(*p)(int,int); // 定义一个函数指针
	
	f p;
	p=fun(100);
	max=p(5,8);
	printf("max=%d\n",max);
	return 0;
}

int getbig(int i,int j)
{
	return i>=j?i:j;
}

//int (*fun(int a))(int i,int j)
f fun(int a)
{
	printf("a=%d\n",a);
	return getbig;
}

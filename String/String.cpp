// String.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//AUTHOR:Nepoleon 
//Date:2021-12-20
#include <iostream>
#include "CString.h"

using namespace std;
int main()
{
	CString str1("Nepoleon");//空
	CString str2(str1);
	str1.ToUpperCase();
	cout << "Str1：" << str1.GetBuf() << endl;
	//str1.Copy("world");
	//str1.Copy("a");
	//str1.Copy("bc");
	//str1.Format("int:%d float:%f", 10, 3.5f);
	//str2.Format("0x%p", &str2);
	str1.Copy("1234");
	str1.Concat("456789");
	cout << "Str1：" << str1.GetBuf() << endl;
	cout << "Str1：" << str1.CharAt(0) << endl;//获取第一个字符
	cout << "Str1：" << str1.LastCharAt(0) << endl;//获取最后一个字符
	cout << "Str2：" << str2.GetBuf() << endl;
	cout << "Str2：" << str2.LastIndexOf('e') << endl;
	
}


///////////////////////////
// by art
//////////////////////////

#include "StdAfx.h"
#include "commFunc.h"

//UTF-8转换到GB3212
string U2G(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	string strRet = str;
	delete []str;
	return strRet;
}

//GB2312到UTF-8的转换
string G2U(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	string strRet = str;
	delete []str;
	return strRet;
}

string GetModuleFilePath()
{
	char szFilePath[MAX_PATH]={0};
	string sModuleFilePath;
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	if(szFilePath != NULL)
	{
		sModuleFilePath=szFilePath;
		int nPos=sModuleFilePath.find_last_of('\\');
		sModuleFilePath = sModuleFilePath.substr(0, nPos);
	}
	else
	{
		sModuleFilePath = "";
	}

	return sModuleFilePath;
}


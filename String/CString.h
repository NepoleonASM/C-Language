#pragma once
class CString
{
public:
	CString();
	~CString();
	CString(const char* pszString);//1.刚开始构造的时候不一定是空的字符串，重载一下这个构造函数
	CString(CString& String);//a.

	void Copy(const char* pszString);
	void Copy(CString& String);//拷贝另外一个字符串？？？？为什么

	void Concat(const char* pszString);
	void Concat(CString& String);//拼接另外一个字符串

	int Compare(const char* pszString);
	int Compare(CString& String);//比较另外一个字符串

	const char* SubString(const char* pszString);
	const char* SubString(CString& String);//字串另外一个字符串

	void SetAt(int nIndex, char ch);//
	char CharAt(int nIndex);  //从前往后找
	char LastCharAt(int nIndex);//从后往前找

	void Split(const char* pszPre);//分割

	void ToUpperCase();
	void ToLowerCase();

	void Format(const char* pszFmt, ...);

	void Replace(char ch1, char ch2);
	void Replace(const char* pszString1, const char* pszString2);

	void Delete(char ch1);
	void Delete(const char* pszString);

	const char* GetBuf();
	inline int Length();

	int IndexOf(char ch);
	int LastIndexOf(char ch);

	bool IsEmpty();//判断是否是空字符串
private:
	char* AllocBuf(int nSize);
	void FreeBuf();
	void InitString(const char* pszString);
private:
	char* m_pBuf;		//缓冲器
	int	  m_nBufSize;	//缓冲区大小 1.
	int	  m_nLength;	//长度
};
int CString::Length()
{
	return m_nLength;
}


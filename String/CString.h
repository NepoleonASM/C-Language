#pragma once
class CString
{
public:
	CString();
	~CString();
	CString(const char* pszString);//1.�տ�ʼ�����ʱ��һ���ǿյ��ַ���������һ��������캯��
	CString(CString& String);//a.

	void Copy(const char* pszString);
	void Copy(CString& String);//��������һ���ַ�����������Ϊʲô

	void Concat(const char* pszString);
	void Concat(CString& String);//ƴ������һ���ַ���

	int Compare(const char* pszString);
	int Compare(CString& String);//�Ƚ�����һ���ַ���

	const char* SubString(const char* pszString);
	const char* SubString(CString& String);//�ִ�����һ���ַ���

	void SetAt(int nIndex, char ch);//
	char CharAt(int nIndex);  //��ǰ������
	char LastCharAt(int nIndex);//�Ӻ���ǰ��

	void Split(const char* pszPre);//�ָ�

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

	bool IsEmpty();//�ж��Ƿ��ǿ��ַ���
private:
	char* AllocBuf(int nSize);
	void FreeBuf();
	void InitString(const char* pszString);
private:
	char* m_pBuf;		//������
	int	  m_nBufSize;	//��������С 1.
	int	  m_nLength;	//����
};
int CString::Length()
{
	return m_nLength;
}


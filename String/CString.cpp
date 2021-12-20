#include "CString.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
CString::CString()
{
    InitString("");
    AllocBuf(1);
    m_pBuf[0] = '\0';
    m_nLength = 0;
}

CString::~CString()
{
    FreeBuf();
}

CString::CString(const char* pszString)//2.
{
    InitString(pszString);
}

CString::CString(CString& String)
{ 
    InitString(String.GetBuf());
}

void CString::Copy(const char* pszString)
{
    int nLen = strlen(pszString);
    //�жϿռ��Ƿ��㹻
    if (nLen >= m_nBufSize)//
    {
        //�ͷ�ԭ���Ŀռ�
        //delete[] m_pBuf;
        FreeBuf();
        //��������ռ�
        AllocBuf(nLen + 1);
    }
    strcpy_s(m_pBuf, m_nBufSize, pszString);//9.
    m_nLength = nLen;//10.
}

void CString::Copy(CString& String)
{
    Copy(String.GetBuf());//11.
}

void CString::Concat(const char* pszString)
{
    int nLen = strlen(pszString);
    int nNeedSize = m_nLength + nLen;
    //�жϿռ��Ƿ��㹻
    if (nNeedSize >= m_nBufSize)//
    {
        //�ȱ���ԭ���Ŀռ䵫�ǲ����ͷ�����char*ǰ��Ҫ��const��������ⶼ��const char*��GetBufҲ��const char*�����Ա�������һ��
        const char* pOldBuf = GetBuf();//

        //���������ڴ�ռ�
        char* pNewBuf = AllocBuf(nNeedSize + 1);

        //�����ɵ�����
        strcpy_s(pNewBuf, m_nBufSize, pOldBuf);

        //�ͷ�ԭ���Ŀռ�
        delete[] pOldBuf;
    }
    strcat_s(m_pBuf, m_nBufSize, pszString);
    m_nLength = nNeedSize;
}

void CString::Concat(CString& String)
{
    Concat(String.GetBuf());
}

int CString::Compare(const char* pszString)
{
    return 0;
}

int CString::Compare(CString& String)
{
    return 0;
}

const char* CString::SubString(const char* pszString)
{
    return nullptr;
}

const char* CString::SubString(CString& String)
{
    return nullptr;
}

void CString::SetAt(int nIndex, char ch)
{
    m_pBuf[nIndex] = ch;
}

char CString::CharAt(int nIndex)
{
    return m_pBuf[nIndex];
}

char CString::LastCharAt(int nIndex)
{
    return m_pBuf[m_nLength - nIndex - 1];
}

void CString::Split(const char* pszPre)
{
}

void CString::ToUpperCase()
{
    for (int i = 0; i < m_nLength; i++)
    {
        if (m_pBuf[i] >= 'a'&& m_pBuf[i]<= 'z')
        {
            m_pBuf[i] -= 32;
        }
    }
}

void CString::ToLowerCase()
{
    for (int i = 0; i < m_nLength; i++)
    {
        if (m_pBuf[i] >= 'A' && m_pBuf[i] <= 'Z')
        {
            m_pBuf[i] += 32;
        }
    }
}

void CString::Format(const char* pszFmt, ...)
{
    char Buf[1024];
    va_list argList;//va_arg ȥmsdn���ѣ�va_list��ȡ�����б�
    va_start(argList, pszFmt);//��Ҫ����ͷ�ļ�stdarg.h
    //vsprintf(Buf, pszFmt, argList);//��Ҫ����stdio.h��stdarg.h
    vsprintf_s(Buf, sizeof(Buf),pszFmt, argList);//��Ҫ����stdio.h��stdarg.h
    va_end(argList);
    Copy(Buf);
}

void CString::Replace(char ch1, char ch2)
{
}

void CString::Replace(const char* pszString1, const char* pszString2)
{
}

void CString::Delete(char ch1)
{
}

void CString::Delete(const char* pszString)
{
}

const char* CString::GetBuf()
{
    if (m_pBuf[0] == '\0')
    {
        return "null";
    }   
    return m_pBuf;
}



int CString::IndexOf(char ch)
{
    for (int i = 0; i < m_nLength; i++)
    {
        if (m_pBuf[i] == ch)
        {
            return i;
        }
    }
    return -1;
}

int CString::LastIndexOf(char ch)
{
    for (int i = m_nLength-1; i >= 0; i--)
    {
        if (m_pBuf[i] == ch)
        {
            return i;
        }
    }
    return -1;
}

bool CString::IsEmpty()
{
    return m_pBuf == nullptr;
}

char * CString::AllocBuf(int nSize)
{
    m_pBuf = new char[nSize];
    m_nBufSize = nSize;
    return m_pBuf;
}

void CString::FreeBuf()
{
    if (m_pBuf != nullptr)
    {
        delete[] m_pBuf;
        m_pBuf = nullptr;
        m_nBufSize = 0;
    }  
}

void CString::InitString(const char* pszString)
{
    m_nLength = strlen(pszString);
    AllocBuf(m_nLength + 1);
    strcpy_s(m_pBuf, m_nBufSize, pszString);
}

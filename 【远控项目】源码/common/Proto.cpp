#include "Proto.h"

int RecvBytes(SOCKET sock, LPBYTE pBuff, DWORD dwLen)
{
    int nRecv = 0;
    while (nRecv < dwLen)
    {
        int nRet = recv(sock, (char*)pBuff + nRecv, dwLen - nRecv, 0);
        if (nRet == 0 || (nRet == SOCKET_ERROR) && WSAGetLastError() == WSAECONNRESET)
        {
            return 0;
        }
        else if (nRet == SOCKET_ERROR)
        {
            return -1;
        }

        nRecv += nRet;
    }

    return dwLen;
}

/*
����ֵ��
 >0 - ���ݽ��ܳɹ�
 0  - ���ӶϿ�
 -1 - ���ݽ��ܴ���
*/
int RecvPack(SOCKET sock, PackageHeader*& pPkg)
{
    //���հ�ͷ
    PackageHeader hdr;
    int nRet = RecvBytes(sock, (LPBYTE)&hdr, sizeof(hdr));
    if (nRet == 0 || nRet == -1)
    {
        return nRet;
    }

    //��������
    char* pBuff = new char[sizeof(PackageHeader) + hdr.m_dwLen];
    if (pBuff == nullptr)
    {
        return -1;
    }
    memcpy(pBuff, &hdr, sizeof(hdr));

    //ֻ�а�ͷ��û������
    if (hdr.m_dwLen <= 0)
    {
        pPkg = (PackageHeader*)pBuff;
        return nRet;
    }

    nRet = RecvBytes(sock, (LPBYTE)pBuff + sizeof(hdr), hdr.m_dwLen);
    if (nRet == 0 || nRet == -1)
    {
        return nRet;
    }

    pPkg = (PackageHeader*)pBuff;
    return sizeof(PackageHeader) + hdr.m_dwLen;
}


int SendPack(SOCKET sock, PackageHeader* pPkg)
{
    //��������
    int nRet = send(sock, (char*)pPkg, sizeof(PackageHeader) + pPkg->m_dwLen, 0);
    if (nRet == SOCKET_ERROR)
    {
        return -1;
    }

    return nRet;
}
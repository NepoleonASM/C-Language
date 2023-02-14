#pragma once


#include <WinSock2.h>
#include <windows.h>

#include <map>
#include <mutex>
#include <thread>
using namespace std;

#include "ByteStreamBuff.h"
#include "CCrc32.h"

enum FLAG
{
    SYN = 0x01,
    ACK = 0x02,
    PSH = 0x04,
    FIN = 0x08
};

#define MAXDATASIZE 1459
#pragma pack(push)
#pragma pack(1)
struct Package
{
    Package() {}

    Package(char flag, int seq, char* buf = nullptr, int len = 0):
        m_chFlag(flag),
        m_nSeq(seq)
    {
        if (buf != nullptr)
        {
            m_nCheck = CCrc32::crc32((unsigned char*)buf, len);//��У��ֵ
            memcpy(m_buf, buf, len);//��������
            m_nLen = len;

        }
    }

    char m_chFlag; //��־
    int   m_nSeq; //���
    int  m_nCheck;//У��ֵ
    int m_nLen;//���ݴ�С
    char m_buf[MAXDATASIZE]; //��������С
};
#pragma pack(pop)
class CUMTSocket
{
public:
    bool  Accept(const char* szIp, unsigned short wPort);
    bool  Connect(const char* szIp, unsigned short wPort);
    int   Recv(char* pBuff, size_t nBufSize);//��������
    int   Send(char* pBuff, size_t nBufSize);//��������
    void  Close();

private:
    void StartWork();

    template<typename... ARGS>
    void Log(ARGS...args)
    {
        char szBuff[MAXBYTE] = {};
        wsprintf(szBuff, args...);
        OutputDebugString(szBuff);
    }


private:

    SOCKET m_sock;

    sockaddr_in m_siSelf;//�󶨵ĵ�ַ��������Ҫ�󶨶˿ں�
    sockaddr_in m_siDst;//Ŀ���ַ���ͻ��˲��ð󶨶˿ں�

    int m_nNextSendSeq = 0;//��һ������seq
    int m_nNextRecvSeq = 0;//��һ���ֵİ���seq

    struct PackageInfo
    {
        PackageInfo(){}
        PackageInfo(bool bSended, time_t tm, const Package& pkg) :
            m_bSended(bSended), m_tmLastTime(tm), m_pkg(pkg)
        {

        }


        bool m_bSended;//���Ƿ��Ѿ����͹�
        time_t m_tmLastTime;//�ϴη��͵�ʱ��
        Package m_pkg;
    };

    map<int, PackageInfo> m_mpSend;//������map,key - seq,val - package
    recursive_mutex m_mtxForSend;//����ͬ��m_mpSend

    map<int, Package> m_mpRecv;//���հ�map,key - seq,val - package
    recursive_mutex m_mtxForRecv;//����ͬ��m_mpRecv

    CByteStreamBuff m_buf;//���ݻ�����
    recursive_mutex m_mtxForBuf;//����ͬ��m_buf

    bool m_bRunning = false;//ѭ���˳��ı�־
    void SendThreadProc();//�����߳�
    void RecvThreadProc();//�հ��߳�
    void HandleThreadProc();//������߳�
};


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
            m_nCheck = CCrc32::crc32((unsigned char*)buf, len);//存校验值
            memcpy(m_buf, buf, len);//拷贝数据
            m_nLen = len;

        }
    }

    char m_chFlag; //标志
    int   m_nSeq; //序号
    int  m_nCheck;//校验值
    int m_nLen;//数据大小
    char m_buf[MAXDATASIZE]; //缓冲区大小
};
#pragma pack(pop)
class CUMTSocket
{
public:
    bool  Accept(const char* szIp, unsigned short wPort);
    bool  Connect(const char* szIp, unsigned short wPort);
    int   Recv(char* pBuff, size_t nBufSize);//接受数据
    int   Send(char* pBuff, size_t nBufSize);//发送数据
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

    sockaddr_in m_siSelf;//绑定的地址、服务器要绑定端口号
    sockaddr_in m_siDst;//目标地址、客户端不用绑定端口号

    int m_nNextSendSeq = 0;//下一个包的seq
    int m_nNextRecvSeq = 0;//下一个手的包的seq

    struct PackageInfo
    {
        PackageInfo(){}
        PackageInfo(bool bSended, time_t tm, const Package& pkg) :
            m_bSended(bSended), m_tmLastTime(tm), m_pkg(pkg)
        {

        }


        bool m_bSended;//包是否已经发送过
        time_t m_tmLastTime;//上次发送的时间
        Package m_pkg;
    };

    map<int, PackageInfo> m_mpSend;//待发包map,key - seq,val - package
    recursive_mutex m_mtxForSend;//用于同步m_mpSend

    map<int, Package> m_mpRecv;//待收包map,key - seq,val - package
    recursive_mutex m_mtxForRecv;//用于同步m_mpRecv

    CByteStreamBuff m_buf;//数据缓冲区
    recursive_mutex m_mtxForBuf;//用于同步m_buf

    bool m_bRunning = false;//循环退出的标志
    void SendThreadProc();//发包线程
    void RecvThreadProc();//收包线程
    void HandleThreadProc();//处理包线程
};


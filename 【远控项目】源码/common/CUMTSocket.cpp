#include "CUMTSocket.h"
#include <time.h>
bool CUMTSocket::Accept(const char* szIp, unsigned short wPort)
{
    m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_sock == INVALID_SOCKET)
    {
        return false;
    }

    m_siSelf.sin_family = AF_INET;
    m_siSelf.sin_port = htons(wPort);
    m_siSelf.sin_addr.S_un.S_addr = inet_addr(szIp);
    int nRet = bind(m_sock, (sockaddr*)&m_siSelf, sizeof(m_siSelf));
    if (nRet == SOCKET_ERROR)
    {
        return false;
    }

    Package pkgRecv;
    int nLen = sizeof(m_siDst);
    nRet = recvfrom(m_sock, (char*)&pkgRecv, sizeof(pkgRecv), 0, (sockaddr*)&m_siDst, &nLen);
    if (nRet == SOCKET_ERROR)
    {
        return false;
    }
    //检查标志
    if (!(pkgRecv.m_chFlag & SYN))//如果不是建立连接SYN标志就返回
    {
        return false;
    }
    Log("[umt] pkg -> SYN seq:%d", m_nNextSendSeq);
    m_nNextRecvSeq = pkgRecv.m_nSeq + 1;//下次序号加1
    //发送ack
    Package pkgSend(ACK | SYN, m_nNextSendSeq++);
    nRet = sendto(m_sock, (char*)&pkgSend, sizeof(pkgSend), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
    //check...

    //接收第三个包
    nRet = recvfrom(m_sock, (char*)&pkgRecv, sizeof(pkgRecv), 0, (sockaddr*)&m_siDst, &nLen);
    //check...
    Log("[umt] pkg -> SYN seq:%d", m_nNextSendSeq);
    //启动工作线程
    StartWork();

    return true;
}

bool CUMTSocket::Connect(const char* szIp, unsigned short wPort)
{
    m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_sock == INVALID_SOCKET)
    {
        return false;
    }

    m_siDst.sin_family = AF_INET;
    m_siDst.sin_port = htons(wPort);
    m_siDst.sin_addr.S_un.S_addr = inet_addr(szIp);
    //发送SYN
    Package pkgSend(SYN, m_nNextSendSeq++);
    int nRet = sendto(m_sock, (char*)&pkgSend, sizeof(pkgSend), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
    //check...
    //收包
    Package pkgRecv;
    int nLen = sizeof(m_siDst);
    nRet = recvfrom(m_sock, (char*)&pkgRecv, sizeof(pkgRecv), 0, (sockaddr*)&m_siDst, &nLen);
    if (nRet == SOCKET_ERROR)
    {
        return false;
    }

    m_nNextRecvSeq = pkgRecv.m_nSeq + 1;//下次序号加1


    //发送第三个包(是第二个包，但是在握手中是第三个包)
    Package pkgAck(ACK, 0);
    nRet = sendto(m_sock, (char*)&pkgAck, sizeof(pkgAck), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
    //check...

    //启动工作线程
    StartWork();

    return true;
}


int CUMTSocket::Send(char* pBuff, size_t nBufSize)
{

    //每次处理一个send
    while (true)
    {
        this_thread::sleep_for(chrono::microseconds(1));//切出线程
        lock_guard<recursive_mutex> lck(m_mtxForSend);
        if (m_mpSend.size() > 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    //计算总共可以拆出多少个包
    size_t nCnt = ((nBufSize % MAXDATASIZE) == 0) ?
        (nBufSize / MAXDATASIZE) ://如果等于零的话呢，说明刚好就是拆这么多个。
        ((nBufSize / MAXDATASIZE) + 1);//否则的话,就再加一个包去处理这个剩余数据。
    //小包放入map
    for (size_t i = 0; i < nCnt; i++)
    {
        lock_guard<recursive_mutex> lck(m_mtxForSend);

        if (i == nCnt - 1)//特殊处理最后一个包
        {
            Log("[umt] pkg -> mpsend seq:%d", m_nNextSendSeq);


            m_mpSend[m_nNextSendSeq] = PackageInfo(false, time(NULL), Package(PSH, m_nNextSendSeq, pBuff +
                i * MAXDATASIZE, nBufSize - i * MAXDATASIZE));//存map
            ++m_nNextSendSeq;

            break;
        }
        Log("[umt] pkg -> mpsend seq:%d", m_nNextSendSeq);
        //构造包
        m_mpSend[m_nNextSendSeq] = PackageInfo(false, time(NULL), Package(PSH, m_nNextSendSeq, pBuff +
            i * MAXDATASIZE, MAXDATASIZE));//存map
        ++m_nNextSendSeq;
    }
    return nBufSize;
}

void CUMTSocket::SendThreadProc()
{
    while (m_bRunning)
    {
        this_thread::sleep_for(chrono::microseconds(1));//切出线程
        lock_guard<recursive_mutex> lck(m_mtxForSend);
        for (auto& pi : m_mpSend)
        {
            if (!pi.second.m_bSended)
            {
                //没有发送过的包，立刻发送
                sendto(m_sock, (char*)&pi.second.m_pkg, sizeof(pi.second.m_pkg), 0,
                    (sockaddr*)&m_siDst, sizeof(m_siDst));
                //更新时间
                pi.second.m_tmLastTime = time(NULL);
                pi.second.m_bSended = true;
                Log("[umt] pkg->network seq:%d", pi.second.m_pkg.m_nSeq);
            }
            else   //发过的包判断是不是超时
            {
                //判断是否超时
                if (time(NULL) - pi.second.m_tmLastTime > 3)
                {
                    //超时的包再发一次
                    sendto(m_sock, (char*)&pi.second.m_pkg, sizeof(pi.second.m_pkg), 0,
                        (sockaddr*)&m_siDst, sizeof(m_siDst));
                    //更新时间
                    pi.second.m_tmLastTime = time(NULL);
                    Log("[umt] pkg->TimeOut seq:%d", pi.second.m_pkg.m_nSeq);
                }
            }
        }
    }

}


int CUMTSocket::Recv(char* pBuff, size_t nBufSize)
{
    while (true)
    {
        this_thread::sleep_for(chrono::microseconds(1));
        lock_guard<recursive_mutex> lck(m_mtxForBuf);
        if (m_buf.GetSize() <= 0)
        {
            continue;
        }
        else
        {
            break;
        }

    }
    //接收数据
    lock_guard<recursive_mutex> lck(m_mtxForBuf);

    size_t nAvailSize = nBufSize >= m_buf.GetSize() ? m_buf.GetSize() : nBufSize;
    m_buf.Read(pBuff, nAvailSize);


    return nAvailSize;
}




void CUMTSocket::Close()
{
}

void CUMTSocket::StartWork()
{
    m_bRunning = true;
    thread(&CUMTSocket::SendThreadProc, this).detach();
    thread(&CUMTSocket::RecvThreadProc, this).detach();
    thread(&CUMTSocket::HandleThreadProc, this).detach();
}


void CUMTSocket::RecvThreadProc()
{
    while (m_bRunning)
    {
        Package pkg;
        sockaddr_in si;
        int nLen = sizeof(si);
        int nRet = recvfrom(m_sock, (char*)&pkg, sizeof(pkg), 0, (sockaddr*)&si, &nLen);
        //check....

        //收到ack，从发包map删除对应seq的包
        if (pkg.m_chFlag & ACK)
        {

            Log("[umt] pkg->Ack seq:%d", pkg.m_nSeq);

            lock_guard<recursive_mutex> lck(m_mtxForSend);
            m_mpSend.erase(pkg.m_nSeq);
            continue;
        }
        //收到psh  2.1 校验成功，发送ack   2.2 校验失败，丢弃
        if (pkg.m_chFlag & PSH)
        {
            //校验
            if (pkg.m_nCheck != CCrc32::crc32((unsigned char*)pkg.m_buf, pkg.m_nLen))
            {
                //校验失败，丢弃
                continue;
            }
            //校验成功，发送ack
            Package pkgAck(ACK, pkg.m_nSeq);
            sendto(m_sock, (char*)&pkgAck, sizeof(pkgAck), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
            Log("[umt] pkg->recvmap seq:%d", pkg.m_nSeq);
            //存入收包map
            lock_guard<recursive_mutex> lck(m_mtxForRecv);
            m_mpRecv[pkg.m_nSeq] = pkg;
        }


    }
    return;
}

void CUMTSocket::HandleThreadProc()
{
    while (m_bRunning)
    {
        //切出线程，他就干这么点活不能让它一直循环，
        this_thread::sleep_for(chrono::microseconds(1));
        
        lock_guard<recursive_mutex> lck(m_mtxForRecv);
        while (m_mpRecv.find(m_nNextRecvSeq) != m_mpRecv.end())
        {
            //包进缓冲区
            Log("[umt] pkg->Buf seq:%d", m_nNextRecvSeq);
            lock_guard<recursive_mutex> lck(m_mtxForBuf);
            m_buf.Write(m_mpRecv[m_nNextRecvSeq].m_buf, m_mpRecv[m_nNextRecvSeq].m_nLen);
            m_mpRecv.erase(m_nNextRecvSeq);
            ++m_nNextRecvSeq;
        }
    }
}

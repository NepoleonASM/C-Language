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
    //����־
    if (!(pkgRecv.m_chFlag & SYN))//������ǽ�������SYN��־�ͷ���
    {
        return false;
    }
    Log("[umt] pkg -> SYN seq:%d", m_nNextSendSeq);
    m_nNextRecvSeq = pkgRecv.m_nSeq + 1;//�´���ż�1
    //����ack
    Package pkgSend(ACK | SYN, m_nNextSendSeq++);
    nRet = sendto(m_sock, (char*)&pkgSend, sizeof(pkgSend), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
    //check...

    //���յ�������
    nRet = recvfrom(m_sock, (char*)&pkgRecv, sizeof(pkgRecv), 0, (sockaddr*)&m_siDst, &nLen);
    //check...
    Log("[umt] pkg -> SYN seq:%d", m_nNextSendSeq);
    //���������߳�
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
    //����SYN
    Package pkgSend(SYN, m_nNextSendSeq++);
    int nRet = sendto(m_sock, (char*)&pkgSend, sizeof(pkgSend), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
    //check...
    //�հ�
    Package pkgRecv;
    int nLen = sizeof(m_siDst);
    nRet = recvfrom(m_sock, (char*)&pkgRecv, sizeof(pkgRecv), 0, (sockaddr*)&m_siDst, &nLen);
    if (nRet == SOCKET_ERROR)
    {
        return false;
    }

    m_nNextRecvSeq = pkgRecv.m_nSeq + 1;//�´���ż�1


    //���͵�������(�ǵڶ��������������������ǵ�������)
    Package pkgAck(ACK, 0);
    nRet = sendto(m_sock, (char*)&pkgAck, sizeof(pkgAck), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
    //check...

    //���������߳�
    StartWork();

    return true;
}


int CUMTSocket::Send(char* pBuff, size_t nBufSize)
{

    //ÿ�δ���һ��send
    while (true)
    {
        this_thread::sleep_for(chrono::microseconds(1));//�г��߳�
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

    //�����ܹ����Բ�����ٸ���
    size_t nCnt = ((nBufSize % MAXDATASIZE) == 0) ?
        (nBufSize / MAXDATASIZE) ://���������Ļ��أ�˵���պþ��ǲ���ô�����
        ((nBufSize / MAXDATASIZE) + 1);//����Ļ�,���ټ�һ����ȥ�������ʣ�����ݡ�
    //С������map
    for (size_t i = 0; i < nCnt; i++)
    {
        lock_guard<recursive_mutex> lck(m_mtxForSend);

        if (i == nCnt - 1)//���⴦�����һ����
        {
            Log("[umt] pkg -> mpsend seq:%d", m_nNextSendSeq);


            m_mpSend[m_nNextSendSeq] = PackageInfo(false, time(NULL), Package(PSH, m_nNextSendSeq, pBuff +
                i * MAXDATASIZE, nBufSize - i * MAXDATASIZE));//��map
            ++m_nNextSendSeq;

            break;
        }
        Log("[umt] pkg -> mpsend seq:%d", m_nNextSendSeq);
        //�����
        m_mpSend[m_nNextSendSeq] = PackageInfo(false, time(NULL), Package(PSH, m_nNextSendSeq, pBuff +
            i * MAXDATASIZE, MAXDATASIZE));//��map
        ++m_nNextSendSeq;
    }
    return nBufSize;
}

void CUMTSocket::SendThreadProc()
{
    while (m_bRunning)
    {
        this_thread::sleep_for(chrono::microseconds(1));//�г��߳�
        lock_guard<recursive_mutex> lck(m_mtxForSend);
        for (auto& pi : m_mpSend)
        {
            if (!pi.second.m_bSended)
            {
                //û�з��͹��İ������̷���
                sendto(m_sock, (char*)&pi.second.m_pkg, sizeof(pi.second.m_pkg), 0,
                    (sockaddr*)&m_siDst, sizeof(m_siDst));
                //����ʱ��
                pi.second.m_tmLastTime = time(NULL);
                pi.second.m_bSended = true;
                Log("[umt] pkg->network seq:%d", pi.second.m_pkg.m_nSeq);
            }
            else   //�����İ��ж��ǲ��ǳ�ʱ
            {
                //�ж��Ƿ�ʱ
                if (time(NULL) - pi.second.m_tmLastTime > 3)
                {
                    //��ʱ�İ��ٷ�һ��
                    sendto(m_sock, (char*)&pi.second.m_pkg, sizeof(pi.second.m_pkg), 0,
                        (sockaddr*)&m_siDst, sizeof(m_siDst));
                    //����ʱ��
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
    //��������
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

        //�յ�ack���ӷ���mapɾ����Ӧseq�İ�
        if (pkg.m_chFlag & ACK)
        {

            Log("[umt] pkg->Ack seq:%d", pkg.m_nSeq);

            lock_guard<recursive_mutex> lck(m_mtxForSend);
            m_mpSend.erase(pkg.m_nSeq);
            continue;
        }
        //�յ�psh  2.1 У��ɹ�������ack   2.2 У��ʧ�ܣ�����
        if (pkg.m_chFlag & PSH)
        {
            //У��
            if (pkg.m_nCheck != CCrc32::crc32((unsigned char*)pkg.m_buf, pkg.m_nLen))
            {
                //У��ʧ�ܣ�����
                continue;
            }
            //У��ɹ�������ack
            Package pkgAck(ACK, pkg.m_nSeq);
            sendto(m_sock, (char*)&pkgAck, sizeof(pkgAck), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
            Log("[umt] pkg->recvmap seq:%d", pkg.m_nSeq);
            //�����հ�map
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
        //�г��̣߳����͸���ô��������һֱѭ����
        this_thread::sleep_for(chrono::microseconds(1));
        
        lock_guard<recursive_mutex> lck(m_mtxForRecv);
        while (m_mpRecv.find(m_nNextRecvSeq) != m_mpRecv.end())
        {
            //����������
            Log("[umt] pkg->Buf seq:%d", m_nNextRecvSeq);
            lock_guard<recursive_mutex> lck(m_mtxForBuf);
            m_buf.Write(m_mpRecv[m_nNextRecvSeq].m_buf, m_mpRecv[m_nNextRecvSeq].m_nLen);
            m_mpRecv.erase(m_nNextRecvSeq);
            ++m_nNextRecvSeq;
        }
    }
}

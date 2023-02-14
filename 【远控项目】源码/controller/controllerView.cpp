
// controllerView.cpp: CcontrollerView 类的实现
//



#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "controller.h"
#endif

#include "controllerDoc.h"
#include "controllerView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif

#include "Proto.h"
#include <thread>
#include "CMyCmd.h"
#include "CMyViewFile.h"
using namespace std;
#define DSTIP "192.168.3.138"    //被控制端IP
//#define DSTIP "127.0.0.1"
//#define DSTIP "10.211.55.5"
// CcontrollerView

IMPLEMENT_DYNCREATE(CcontrollerView, CView)

BEGIN_MESSAGE_MAP(CcontrollerView, CView)
    // 标准打印命令
    ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
    ON_COMMAND(MN_CON_CREEN_START, &CcontrollerView::OnConCreenStart)
    ON_COMMAND(MN_CON_START, &CcontrollerView::OnConStart)
    ON_WM_TIMER()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDBLCLK()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_COMMAND(MN_CON_CREEN_CLOSE, &CcontrollerView::OnConCreenClose)
    ON_COMMAND(MN_EX_CMD, &CcontrollerView::OnExCmd)
    ON_COMMAND(MN_FILE_VIEW, &CcontrollerView::OnFileView)
END_MESSAGE_MAP()

// CcontrollerView 构造/析构

CcontrollerView::CcontrollerView() noexcept
{
    // TODO: 在此处添加构造代码

}



CcontrollerView::~CcontrollerView()
{
}

BOOL CcontrollerView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: 在此处通过修改
    //  CREATESTRUCT cs 来修改窗口类或样式

    return CView::PreCreateWindow(cs);
}

// CcontrollerView 绘图

void CcontrollerView::OnDraw(CDC* pDC)
{
    CcontrollerDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;

    // TODO: 在此处为本机数据添加绘制代码
                //拷贝图片数据到客户区
    lock_guard<recursive_mutex> lck(m_mtxFoSD);
    if (m_ptrSDPkg == nullptr)
    {
        return;
    }
    CDC dcMem;
    dcMem.CreateCompatibleDC(pDC);//创建内存dc            
    CBitmap bmpMem;
    bmpMem.CreateCompatibleBitmap(pDC, m_ptrSDPkg->m_dwWith, m_ptrSDPkg->m_dwHeight);//创建内存位图
    bmpMem.SetBitmapBits(m_ptrSDPkg->m_dwScreenDataLen, m_ptrSDPkg->m_bufScreenData);//前面是大小，后面是缓冲区
    dcMem.SelectObject(&bmpMem);//将内存位图选入内存DC
    pDC->BitBlt(0, 0, m_ptrSDPkg->m_dwWith, m_ptrSDPkg->m_dwHeight,
        &dcMem, 0, 0,
        SRCCOPY);//拷贝到客户区



}


// CcontrollerView 打印

BOOL CcontrollerView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // 默认准备
    return DoPreparePrinting(pInfo);
}

void CcontrollerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: 添加额外的打印前进行的初始化过程
}

void CcontrollerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: 添加打印后进行的清理过程
}


// CcontrollerView 诊断

#ifdef _DEBUG
void CcontrollerView::AssertValid() const
{
    CView::AssertValid();
}

void CcontrollerView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CcontrollerDoc* CcontrollerView::GetDocument() const // 非调试版本是内联的
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CcontrollerDoc)));
    return (CcontrollerDoc*)m_pDocument;
}
#endif //_DEBUG


// CcontrollerView 消息处理程序

void CcontrollerView::OnConStart()
{
    //连接服务器
    //创建socket
    m_sock = socket(AF_INET,    //协议族
        SOCK_STREAM,            //数据流
        IPPROTO_TCP             //tcp协议
    );
    if (m_sock == INVALID_SOCKET)
    {
        AfxMessageBox("socket failed");
        return;
    }

    //连接服务器
    sockaddr_in si;
    si.sin_family = AF_INET;
    si.sin_port = htons(0x9527);
    si.sin_addr.S_un.S_addr = inet_addr(DSTIP);
    //si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    int nRet = connect(m_sock, (sockaddr*)&si, sizeof(si));
    if (nRet == SOCKET_ERROR)
    {
        AfxMessageBox("connect failed");
        return;
    }

    m_bRuning = TRUE;
    thread(&CcontrollerView::WorkThread, this).detach();

    AfxMessageBox("连接被控端成功");
}

void CcontrollerView::OnConCreenStart()
{
    SetTimer(1, 800, NULL);
}




void CcontrollerView::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    auto ptrPkg = make_package<C2SScreenDataPkg>(sizeof(C2SScreenDataPkg));
    SendPack(m_sock, ptrPkg.get());


    CView::OnTimer(nIDEvent);
}
void CcontrollerView::WorkThread()
{
    while (m_bRuning)
    {
        PackageHeader* pPkg = nullptr;
        int nRet = RecvPack(m_sock, pPkg);
        if (nRet == 0)
        {
            OutputDebugString("connection disconnected or error ... \r\n");
            return;
        }
        else if (nRet == -1)
        {
            OutputDebugString("RecvPack failed  ... \r\n");
            continue;
        }

        switch (pPkg->m_pc)
        {
        case S2C_SCREENDATA:
        {
            S2CScreenDataPkg* pSDPkg = (S2CScreenDataPkg*)pPkg;
            m_mtxFoSD.lock();
            m_ptrSDPkg = shared_ptr<S2CScreenDataPkg>(pSDPkg);
            m_mtxFoSD.unlock();
            InvalidateRect(NULL, FALSE);//是否刷新背景

            pPkg = NULL;
            break;
        }
        case S2C_CMD:
        {
            S2CCmdPkg* pS2CCMDPkg = (S2CCmdPkg*)pPkg;
            m_strShow += "\r\n";
            m_strShow += pS2CCMDPkg->m_bufCmdDate;

            HWND hCmd = ::FindWindow(NULL, "CMD命令行");
            ::SetDlgItemText(hCmd, EDT_SHOW, m_strShow.GetBuffer());
            break;
        }
        case S2C_FILE:
        {
            S2CFilePkg* pS2CFilePkg = (S2CFilePkg*)pPkg;

            //获取其它窗口对象
            //HWND hFile = ::FindWindow(NULL, "文件浏览工具");
            //CMyViewFile* pWnd = (CMyViewFile*)CMyViewFile::FromHandle(hFile);

            //判断是否是盘符、传过来的盘符m_bufFileDate是NULL，长度是0
            if (strlen(pS2CFilePkg->m_bufFileDate) == 0)
            {
                //设置第一列的数据
                pWnd->m_lstcViewFile.InsertItem(pS2CFilePkg->m_nRow, pS2CFilePkg->m_bufFilePos);
                pWnd->m_lstcViewFile.SetItemData(pS2CFilePkg->m_nRow, pS2CFilePkg->m_nCol);

                //设置第二列的数据
                pWnd->m_lstcViewFile.SetItemText(pS2CFilePkg->m_nRow, 1,
                    pS2CFilePkg->m_szFileType);
                //设置第三列的数据
                pWnd->m_lstcViewFile.SetItemText(pS2CFilePkg->m_nRow, 2,
                    pS2CFilePkg->m_szFileSize);
            }
            else
            {
                pWnd->m_lstcViewFile.InsertItem(pS2CFilePkg->m_nRow, pS2CFilePkg->m_bufFileDate);
                pWnd->m_lstcViewFile.SetItemData(pS2CFilePkg->m_nRow, pS2CFilePkg->m_nCol);
                pWnd->m_lstcViewFile.SetItemText(pS2CFilePkg->m_nRow, 1,
                    pS2CFilePkg->m_szFileType);
                pWnd->m_lstcViewFile.SetItemText(pS2CFilePkg->m_nRow, 2,
                    pS2CFilePkg->m_szFileSize);
            }
            //显示当前的路径
            pWnd->m_strPosition = pS2CFilePkg->m_bufFilePos;
            break;
        }
        case S2C_FILE_DOWNLOAD:
        {
            S2CFileDownloadPkg* pS2CFilePkg = (S2CFileDownloadPkg*)pPkg;
            //如果m_cType是1表示发过来的是文件的长度，否则的话就是数据
            if (pS2CFilePkg->m_cType == 1)
            {
                m_nSumSize = pS2CFilePkg->m_nSize;
                m_nSize = 0;
                m_hFile = CreateFile(m_strStoreFilePath,           // open MYFILE.TXT 
                    GENERIC_WRITE,              // open for reading 
                    FILE_SHARE_READ,           // share for reading 
                    NULL,                      // no security 
                    CREATE_ALWAYS,             // existing file only 
                    FILE_ATTRIBUTE_NORMAL,     // normal file 
                    NULL);                     // no attr. template 

                if (m_hFile == INVALID_HANDLE_VALUE)
                {
                    printf("Could not open file.\r\n");   // process error 
                    return;
                }
            }
            else
            {
                if (m_nSize < m_nSumSize)
                {
                    DWORD dwBytesWrited = 0;
                    WriteFile(m_hFile, pS2CFilePkg->m_bufFileDownloadDate, 
                        pS2CFilePkg->m_dwFileDownloadDataLen, &dwBytesWrited, NULL);//第三个参数表示收多少就写多少。
                    m_nSize += dwBytesWrited;
                    if (m_nSize >= m_nSumSize)
                    {
                        CloseHandle(m_hFile);
                        AfxMessageBox("文件下载完毕");
                    }
                }
            }
            break;
        }
        default:
            break;
        }

        if (pPkg != NULL)
        {
            delete (char*)pPkg;//因为RecvPack的时候new了、所以这边delete一下
        }
    }

}

//判断鼠标键盘事件是否被按下
BOOL CcontrollerView::PreTranslateMessage(MSG* pMsg)
{
    
    if (pMsg->message == WM_KEYDOWN)
    {
        auto ptrPkg = make_package<C2SKeybdPkg>(sizeof(C2SKeybdPkg), pMsg->wParam);
        SendPack(m_sock, ptrPkg.get());
    }
    return CView::PreTranslateMessage(pMsg);
}

//鼠标移动
void CcontrollerView::OnMouseMove(UINT nFlags, CPoint point)
{
    
    auto ptrPkg = make_package<C2SMousebdPkg>(sizeof(C2SMousebdPkg), MOUSEEVENTF_MOVE, point.x, point.y);
    SendPack(m_sock, ptrPkg.get());
    CView::OnMouseMove(nFlags, point);
}

//左键按下
void CcontrollerView::OnLButtonDown(UINT nFlags, CPoint point)
{
    
    auto ptrPkg = make_package<C2SMousebdPkg>(sizeof(C2SMousebdPkg), MOUSEEVENTF_LEFTDOWN, point.x, point.y);
    SendPack(m_sock, ptrPkg.get());
    CView::OnLButtonDown(nFlags, point);
}

//左键按下
void CcontrollerView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
   
    auto ptrPkg = make_package<C2SMousebdPkg>(sizeof(C2SMousebdPkg), MOUSEEVENTF_LEFTDOWN, point.x, point.y);
    SendPack(m_sock, ptrPkg.get());
    CView::OnLButtonDblClk(nFlags, point);
}

//左键弹起
void CcontrollerView::OnLButtonUp(UINT nFlags, CPoint point)
{
  
    auto ptrPkg = make_package<C2SMousebdPkg>(sizeof(C2SMousebdPkg), MOUSEEVENTF_LEFTUP, point.x, point.y);
    SendPack(m_sock, ptrPkg.get());
    CView::OnLButtonUp(nFlags, point);
}

//
void CcontrollerView::OnRButtonDblClk(UINT nFlags, CPoint point)
{
    CView::OnRButtonDblClk(nFlags, point);
}

//右键按下
void CcontrollerView::OnRButtonDown(UINT nFlags, CPoint point)
{
   
    auto ptrPkg = make_package<C2SMousebdPkg>(sizeof(C2SMousebdPkg), MOUSEEVENTF_RIGHTDOWN, point.x, point.y);
    SendPack(m_sock, ptrPkg.get());
    CView::OnRButtonDown(nFlags, point);
}

//右键弹起
void CcontrollerView::OnRButtonUp(UINT nFlags, CPoint point)
{
    
    auto ptrPkg = make_package<C2SMousebdPkg>(sizeof(C2SMousebdPkg), MOUSEEVENTF_RIGHTUP, point.x, point.y);
    SendPack(m_sock, ptrPkg.get());
    CView::OnRButtonUp(nFlags, point);
}
//关闭屏幕数据
void CcontrollerView::OnConCreenClose()
{
    KillTimer(1);
   
}

//执行cmd
void CcontrollerView::OnExCmd()
{
    CMyCmd dlg;
    dlg.m_socket = m_sock;//将主窗口的socket传到cmd窗口
    dlg.DoModal();
}

//执行文件浏览
void CcontrollerView::OnFileView()
{
    CMyViewFile dlg;
    pWnd = &dlg;                //保存dialog的对象指针
    dlg.m_sock = m_sock;
    dlg.m_controllerView = this;//把view窗口的this指针传到对话框中
    dlg.DoModal();
}

// CMyViewFile.cpp: 实现文件
//

#include "pch.h"
#include "controller.h"
#include "CMyViewFile.h"
#include "afxdialogex.h"
#include "controllerView.h"
#include "Proto.h"
#include "CMyDownload.h"
#include "CMyUpload.h"

// CMyViewFile 对话框

IMPLEMENT_DYNAMIC(CMyViewFile, CDialogEx)

CMyViewFile::CMyViewFile(CWnd* pParent /*=nullptr*/)
    : CDialogEx(DLG_FILE, pParent)
    , m_strPosition(_T(""))
{

}

CMyViewFile::~CMyViewFile()
{
}

void CMyViewFile::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, EDT_POSITION, m_strPosition);
    DDX_Control(pDX, LSTC_FILE_VIEW, m_lstcViewFile);
}


BEGIN_MESSAGE_MAP(CMyViewFile, CDialogEx)
    ON_BN_CLICKED(BTN_REDIRECT, &CMyViewFile::OnBnClickedRedirect)
    ON_NOTIFY(NM_DBLCLK, LSTC_FILE_VIEW, &CMyViewFile::OnDblclkLstcFileView)
    ON_WM_RBUTTONDOWN()
    ON_NOTIFY(NM_RCLICK, LSTC_FILE_VIEW, &CMyViewFile::OnRclickLstcFileView)
    ON_COMMAND(MN_DOWNLOAD, &CMyViewFile::OnDownload)
    ON_COMMAND(MN_UPLOAD, &CMyViewFile::OnUpload)
END_MESSAGE_MAP()


// CMyViewFile 消息处理程序


BOOL CMyViewFile::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    while (m_lstcViewFile.DeleteColumn(0));
    m_lstcViewFile.DeleteAllItems();
    int nIdx = 0;
    //m_strPosition += "C:";
    m_lstcViewFile.InsertColumn(nIdx++, "文件名", LVCFMT_LEFT, 400);
    m_lstcViewFile.InsertColumn(nIdx++, "类型", LVCFMT_LEFT, 200);
    m_lstcViewFile.InsertColumn(nIdx++, "文件大小", LVCFMT_LEFT, 100);
    m_lstcViewFile.SetExtendedStyle(
        m_lstcViewFile.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    UpdateData(FALSE);





    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}




void CMyViewFile::OnBnClickedRedirect()
{
    UpdateData(TRUE);
    m_lstcViewFile.DeleteAllItems();//每次跳转的时候把list控件清空
    auto pC2SFilePkg = make_package<C2SFilePkg>(sizeof(C2SFilePkg),
        m_strPosition.GetBuffer());
    SendPack(m_sock, pC2SFilePkg.get());
}


void CMyViewFile::TravelFile()
{
    UpdateData(TRUE);
    int nRow = 0;
    int nCol = 0;
    int iIdx = 1;
    CString strSize;
    CFileFind finder;
    m_strPosition += "\\*";
    m_lstcViewFile.DeleteAllItems();
    BOOL bWorking = finder.FindFile(m_strPosition.GetBuffer());
    while (bWorking)
    {
        nCol = 1;
        bWorking = finder.FindNextFile();
        iIdx = m_lstcViewFile.InsertItem(nRow, finder.GetFileName());
        if (finder.IsDirectory())
        {
            if (finder.GetFileName().Compare("..") == 0)
            {
                m_lstcViewFile.SetItemText(nRow, nCol++, "返回上一层");
                strSize.Format("%d KB", finder.GetLength() / 1024);
                m_lstcViewFile.SetItemText(nRow++, nCol++, strSize.GetBuffer());
            }
            else
            {
                m_lstcViewFile.SetItemText(nRow, nCol++, "文件夹");
                strSize.Format("%d KB", finder.GetLength() / 1024);
                m_lstcViewFile.SetItemText(nRow++, nCol++, strSize.GetBuffer());
            }

            m_lstcViewFile.SetItemData(iIdx, 1);

        }
        else
        {
            //获取文件类型  
            int nLen = finder.GetFileName().GetLength();
            int nSplitLen = finder.GetFileName().ReverseFind('.');
            string strIniPath = finder.GetFileName().Right(nLen - nSplitLen - 1);
            strIniPath += "类型文件";

            m_lstcViewFile.SetItemText(nRow, nCol++, strIniPath.c_str());
            strSize.Format("%d KB", finder.GetLength() / 1024);
            m_lstcViewFile.SetItemText(nRow++, nCol++, strSize.GetBuffer());
            m_lstcViewFile.SetItemData(iIdx, 0);
        }

    }

    UpdateData(FALSE);
}


void CMyViewFile::OnDblclkLstcFileView(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    *pResult = 0;
    int nIdxSel = m_lstcViewFile.GetSelectionMark();//获取选中项
    if (nIdxSel == -1)//如果选择空白就返回
    {
        return;
    }

    if (m_lstcViewFile.GetItemData(nIdxSel) == 0)
    {
        UpdateData(FALSE);
        return;
    }
    //获取所选路径
    UpdateData(FALSE);
    m_strPosition = m_strPosition.Left(m_strPosition.GetLength() - 1);//将*去掉

    CString strDoubleIdot = m_lstcViewFile.GetItemText(nIdxSel, 0);
    //如果是..就返回上一层
    if (strDoubleIdot.Compare("..") == 0)
    {
        //如果是..就返回上一层、同时更改一下路径
        int n = m_strPosition.ReverseFind('\\');
        m_strPosition.SetAt(n, '\0');
        n = m_strPosition.ReverseFind('\\');
        m_strPosition.SetAt(n, '\0');

        UpdateData(FALSE);

        auto pC2SFilePkg = make_package<C2SFilePkg>(sizeof(C2SFilePkg),
            m_strPosition.GetBuffer());
        SendPack(m_sock, pC2SFilePkg.get());
        m_lstcViewFile.DeleteAllItems();
        return;
    }
    if (m_lstcViewFile.GetItemData(nIdxSel) == 2)
    {
        //如果是根路径就把，当前显示的路径清空
        m_strPosition.Empty();
    }
    //拼接当前的文件路径
    m_strPosition += m_lstcViewFile.GetItemText(nIdxSel, 0);
    UpdateData(FALSE);



    auto pC2SFilePkg = make_package<C2SFilePkg>(sizeof(C2SFilePkg),
        m_strPosition.GetBuffer());
    SendPack(m_sock, pC2SFilePkg.get());
    m_lstcViewFile.DeleteAllItems();
}


void CMyViewFile::OnRButtonDown(UINT nFlags, CPoint point)
{
    CDialogEx::OnRButtonDown(nFlags, point);
}


void CMyViewFile::OnRclickLstcFileView(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    *pResult = 0;

    //右键弹出菜单
    CMenu menu;
    menu.LoadMenu(IDR_MAINFRAME);
    CMenu* pPopup = menu.GetSubMenu(5);
    CPoint pointPos;
    GetCursorPos(&pointPos);
    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pointPos.x, pointPos.y, this);
}


void CMyViewFile::OnDownload()
{
    CMyDownload dlg;


    dlg.m_ViewFileThis = this;//将this指针保存到文件下载对话框
    dlg.m_sock = m_sock;      //将sock保存到文件浏览类
    dlg.DoModal();

}


void CMyViewFile::OnUpload()
{
    CMyUpload dlg;
    dlg.m_ViewFileThis = this;//将this指针保存到文件上传对话框
    dlg.m_sock = m_sock;      //将sock保存到文件浏览类
    dlg.DoModal();
}

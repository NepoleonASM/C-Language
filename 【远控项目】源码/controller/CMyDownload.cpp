// CMyDownload.cpp: 实现文件
//

#include "pch.h"
#include "controller.h"
#include "CMyDownload.h"
#include "afxdialogex.h"
#include <Proto.h>


// CMyDownload 对话框

IMPLEMENT_DYNAMIC(CMyDownload, CDialogEx)

CMyDownload::CMyDownload(CWnd* pParent /*=nullptr*/)
    : CDialogEx(DLG_DOWNLOAD, pParent)
{

}

CMyDownload::~CMyDownload()
{
}

void CMyDownload::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMyDownload, CDialogEx)
    ON_BN_CLICKED(BTN_DOWNLOAD, &CMyDownload::OnBnClickedDownload)
    ON_BN_CLICKED(BTN_SELETC_FOLDER, &CMyDownload::OnBnClickedSeletcFolder)
END_MESSAGE_MAP()


// CMyDownload 消息处理程序
#include "controllerView.h"

void CMyDownload::OnBnClickedDownload()
{
    UpdateData(TRUE);
    CString  strStoreFilePath;      //保存文件的路径
    CString  strFileDownloadPath;   //目标机器的文件路径
    CString  strFileName;           //文件名

    //拿下载路径
    int nIdex = m_ViewFileThis->m_lstcViewFile.GetSelectionMark();
    //获取文件名字
    strFileName = m_ViewFileThis->m_lstcViewFile.GetItemText(nIdex, 0);
    //拿存储路径
    GetDlgItemText(EDT_FILEPATH, strStoreFilePath);
    m_ViewFileThis->m_controllerView->m_strStoreFilePath = strStoreFilePath + "\\" + strFileName;
    //获取整个下载路径
    strFileDownloadPath = m_ViewFileThis->m_strPosition;
    strFileDownloadPath.Replace("\\\\", "\\");
    strFileDownloadPath.Replace("\\*", "\\");
    strFileDownloadPath += strFileName;
    //将下载路径发给服务器
    auto pkgC2SFilePath = make_package<C2SFileDownloadPkg>(sizeof(C2SFileDownloadPkg) +
        strFileDownloadPath.GetLength() + 1, (LPBYTE)strFileDownloadPath.GetBuffer(),
        strFileDownloadPath.GetLength() + 1);
    SendPack(m_sock, pkgC2SFilePath.get());
    EndDialog(0);
}


void CMyDownload::OnBnClickedSeletcFolder()
{
    //选择存储路径
    CString  strFilePath;
    TCHAR   szPath[MAX_PATH] = { 0 };

    LPITEMIDLIST   pitem;

    BROWSEINFO   info;

    ::ZeroMemory(&info, sizeof(info));

    info.hwndOwner = this->m_hWnd;

    info.lpszTitle = _T("Choose   the   Folder: ");

    info.pszDisplayName = szPath;

    if (pitem = ::SHBrowseForFolder(&info))

    {

        ::SHGetPathFromIDList(pitem, szPath);

        strFilePath = szPath;

        UpdateData(FALSE);

    }
    SetDlgItemText(EDT_FILEPATH, strFilePath.GetBuffer());

}

// CMyUpload.cpp: 实现文件
//

#include "pch.h"
#include "controller.h"
#include "CMyUpload.h"
#include "afxdialogex.h"
#include "resource.h"
#include <Proto.h>

// CMyUpload 对话框

IMPLEMENT_DYNAMIC(CMyUpload, CDialogEx)

CMyUpload::CMyUpload(CWnd* pParent /*=nullptr*/)
    : CDialogEx(DLG_UPLOAD, pParent)
{

}

CMyUpload::~CMyUpload()
{
}

void CMyUpload::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_ebcUpload);
}


BEGIN_MESSAGE_MAP(CMyUpload, CDialogEx)
    ON_BN_CLICKED(BTN_OK, &CMyUpload::OnBnClickedOk)
END_MESSAGE_MAP()


// CMyUpload 消息处理程序


void CMyUpload::OnBnClickedOk()
{
    CString strFilePath;
    GetDlgItemText(EDTB_FILE, strFilePath);

    HANDLE hFile;

    hFile = CreateFile(strFilePath,           // open MYFILE.TXT 
        GENERIC_READ,              // open for reading 
        FILE_SHARE_READ,           // share for reading 
        NULL,                      // no security 
        OPEN_EXISTING,             // existing file only 
        FILE_ATTRIBUTE_NORMAL,     // normal file 
        NULL);                     // no attr. template 

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Could not open file.\r\n");   // process error 
        return;
    }
    //拿文件大小
    DWORD dwSize = GetFileSize(hFile, NULL);
    //发送文件大小和文件名字
    CString strDstFilePath;
    strDstFilePath = m_ViewFileThis->m_strPosition;
    //将文件名字分割出来、并拼接到要上传的文件路径
    strDstFilePath.Replace("\\\\", "\\");
    strDstFilePath.Replace("\\*", "\\");

    strDstFilePath += strFilePath.Right(strFilePath.GetLength() - strFilePath.ReverseFind('\\') - 1);

    auto pkgS2CFileUploadSize = make_package<C2SFileUploadPkg>(sizeof(C2SFileUploadPkg) +
        strDstFilePath.GetLength() + 1,
        (LPBYTE)strDstFilePath.GetBuffer(), strDstFilePath.GetLength() + 1, 1, dwSize);
    SendPack(m_sock, pkgS2CFileUploadSize.get());

    char buf[0x1000] = {};

    int nSize = 0;
    while (true)
    {
        DWORD dwBytesReaded = 0;
        ReadFile(hFile, buf, sizeof(buf), &dwBytesReaded, NULL);
        if (dwBytesReaded <= 0)
        {
            CloseHandle(hFile);
            return;
        }
        auto pkgS2CFileUpload = make_package<C2SFileUploadPkg>(sizeof(C2SFileUploadPkg) + dwBytesReaded,
            (LPBYTE)buf, dwBytesReaded, 2, 0);
        SendPack(m_sock, pkgS2CFileUpload.get());
        nSize += dwBytesReaded;
        if (nSize >= dwSize)
        {
            CloseHandle(hFile);
            AfxMessageBox("上传完毕");
            EndDialog(0);
            return;
        }
    }

}

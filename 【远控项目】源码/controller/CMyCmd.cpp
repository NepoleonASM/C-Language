// CMyCmd.cpp: 实现文件
//

#include "pch.h"
#include "controller.h"
#include "CMyCmd.h"
#include "afxdialogex.h"
#include "Proto.h"

// CMyCmd 对话框

IMPLEMENT_DYNAMIC(CMyCmd, CDialogEx)

CMyCmd::CMyCmd(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DLG_CMD, pParent)
    , m_strCmd(_T(""))
    
{

}

CMyCmd::~CMyCmd()
{
}

void CMyCmd::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, EDT_CMD, m_strCmd);
    
}


BEGIN_MESSAGE_MAP(CMyCmd, CDialogEx)
    ON_BN_CLICKED(BTN_EXECUTE, &CMyCmd::OnBnClickedExecute)
END_MESSAGE_MAP()


// CMyCmd 消息处理程序


void CMyCmd::OnBnClickedExecute()
{
    UpdateData(TRUE);
    
    m_strCmd += "\r\n";
    m_strCmd = "\r\n" + m_strCmd;//前面加上\r\n保险一点
    auto pkgC2S = make_package<C2SCmdPkg>(sizeof(C2SCmdPkg) + m_strCmd.GetLength(),
        (LPBYTE)m_strCmd.GetBuffer(), m_strCmd.GetLength());
    SendPack(m_socket, pkgC2S.get());
}


BOOL CMyCmd::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    m_strCmd = "\r\n" + m_strCmd;//前面加上\r\n保险一点
    auto pkgC2S = make_package<C2SCmdPkg>(sizeof(C2SCmdPkg),
        (LPBYTE)m_strCmd.GetBuffer(), 0);
    SendPack(m_socket, pkgC2S.get());
    
    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}

#pragma once


// CMyCmd 对话框

class CMyCmd : public CDialogEx
{
	DECLARE_DYNAMIC(CMyCmd)

public:
	CMyCmd(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMyCmd();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DLG_CMD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    CString m_strCmd;//cmd命令
    
    afx_msg void OnBnClickedExecute();
    virtual BOOL OnInitDialog();
    SOCKET m_socket;//view的socket

private:
    //管道
    HANDLE m_hRead;
    HANDLE m_hCmdWrite;
    HANDLE m_hWrite;
    HANDLE m_hCmdRead;
};

#pragma once
#include "CMyViewFile.h"

// CMyDownload 对话框

class CMyDownload : public CDialogEx
{
	DECLARE_DYNAMIC(CMyDownload)

public:
	CMyDownload(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMyDownload();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DLG_DOWNLOAD };
#endif

public:
    
public:
    SOCKET m_sock;                     //view的socket
    CMyViewFile* m_ViewFileThis;      //view的this
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedDownload();
    afx_msg void OnBnClickedSeletcFolder();
};

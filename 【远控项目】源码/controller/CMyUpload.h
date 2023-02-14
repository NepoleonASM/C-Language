#pragma once
#include "CMyViewFile.h"


// CMyUpload 对话框

class CMyUpload : public CDialogEx
{
	DECLARE_DYNAMIC(CMyUpload)

public:
	CMyUpload(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMyUpload();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DLG_UPLOAD };
#endif
public:
    SOCKET m_sock;                    //view的socket
    CMyViewFile* m_ViewFileThis;      //view的this指针
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    CMFCEditBrowseCtrl m_ebcUpload;   //文件浏览控件
    afx_msg void OnBnClickedOk();
};

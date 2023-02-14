#pragma once


// CMyViewFile 对话框
class CcontrollerView;
class CMyViewFile : public CDialogEx
{
	DECLARE_DYNAMIC(CMyViewFile)

public:
	CMyViewFile(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMyViewFile();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DLG_FILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    SOCKET m_sock;                      //view的socket
    CString m_strPosition;              //当前位置路径
    CListCtrl m_lstcViewFile;           
    CcontrollerView* m_controllerView;  //view的this指针


    virtual BOOL OnInitDialog();
    afx_msg void OnFileView();
    afx_msg void OnBnClickedRedirect();
    void TravelFile();
    afx_msg void OnDblclkLstcFileView(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRclickLstcFileView(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDownload();
    afx_msg void OnUpload();
};


// controllerView.h: CcontrollerView 类的接口
//

#pragma once
#include <mutex>
using namespace std;
#include "Proto.h"
#include "controllerDoc.h"
class CMyViewFile;

class CcontrollerView : public CView
{
protected: // 仅从序列化创建
    CcontrollerView() noexcept;
    DECLARE_DYNCREATE(CcontrollerView)

private:
    SOCKET m_sock;                              //保存socket
    shared_ptr<S2CScreenDataPkg> m_ptrSDPkg;    //屏幕数据包的指针
    recursive_mutex m_mtxFoSD;                  //用来同步屏幕数据

    BOOL m_bRuning = FALSE;                     //循环的标志
    void WorkThread();



    // 特性
public:
    CcontrollerDoc* GetDocument() const;

    // 操作
public:
    CString m_strShow;
    CMyViewFile* m_pViewFile;
    // 重写
public:
    virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

    // 实现
public:
    virtual ~CcontrollerView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
public:
    CMyViewFile* pWnd;            //文件浏览窗口的this指针
    CString  m_strStoreFilePath;  //文件存储路径
    int m_nSumSize;               //文件的总大小
    int m_nSize;                  //已经收到的文件大小
    HANDLE m_hFile;               //文件句柄
protected:

    // 生成的消息映射函数
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnConCreenStart();
    afx_msg void OnConStart();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnConCreenClose();
    afx_msg void OnExCmd();
    afx_msg void OnFileView();
};

#ifndef _DEBUG  // controllerView.cpp 中的调试版本
inline CcontrollerDoc* CcontrollerView::GetDocument() const
{
    return reinterpret_cast<CcontrollerDoc*>(m_pDocument);
}
#endif


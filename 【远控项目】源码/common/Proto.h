
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <afx.h>
#include <winsock2.h>
#include <Windows.h>
#include <memory>

using namespace std;

enum ProtoCommand
{
    C2S_SCREENDATA,     //请求屏幕数据
    C2S_KEY,            //请求键盘控制
    C2S_MOUSE,          //请求鼠标控制
    C2S_CMD,            //请求执行cmd命令
    C2S_FILE,           //请求浏览文件
    C2S_FILE_DOWNLOAD,  //请求文件下载
    C2S_FILE_UPLOAD,    //请求文件上传
    //...

    S2C_SCREENDATA,     //返回屏幕数据
    S2C_CMD,            //返回cmd命令结果
    S2C_FILE,           //返回文件数据
    S2C_FILE_DOWNLOAD,  //返回文件下载数据
    S2C_FILE_UPLOAD     //返回文件上传

    //...
};

#pragma pack(push)
#pragma pack(1)
struct PackageHeader
{
    PackageHeader() {}
    PackageHeader(ProtoCommand pc, DWORD dwLen) :m_pc(pc), m_dwLen(dwLen) {}

    ProtoCommand m_pc; //包类型
    DWORD m_dwLen;     //数据大小
};

struct C2SFilePkg :public PackageHeader
{
    C2SFilePkg() :
        PackageHeader(C2S_FILE, 0) {}
    C2SFilePkg(char* pBuff)
        :PackageHeader(C2S_FILE, sizeof(*this) - sizeof(PackageHeader))
    {
        strcpy(m_bufFilePos, pBuff);
    }
    char m_bufFilePos[1024];//当前文件所在的路径
};


struct S2CFilePkg :public PackageHeader
{
    S2CFilePkg() :
        PackageHeader(S2C_FILE, 0),
        m_dwFileDataLen(0), m_nRow(0), m_nCol(0)
    {}
    S2CFilePkg(char* pFileType, char* szFileSize, char* pPosBuff,
        char* pDataBuff, DWORD dwDataLen, int nRow, int nCol)
        :PackageHeader(S2C_FILE, sizeof(*this) - sizeof(PackageHeader) + dwDataLen)
    {
        m_nRow = nRow;
        m_nCol = nCol;
        memcpy(m_szFileType, pFileType, sizeof(m_szFileType));
        memcpy(m_szFileSize, szFileSize, sizeof(m_szFileSize));
        memcpy(m_bufFilePos, pPosBuff, sizeof(m_bufFilePos));
        //strcpy(m_szFileSize, szFileSize);
        //strcpy(m_bufFilePos, pPosBuff);
        m_dwFileDataLen = dwDataLen;
        memcpy(m_bufFileDate, pDataBuff, m_dwFileDataLen);
    }
    char m_szFileType[64];//文件类型
    char m_szFileSize[64];//文件大小
    char m_bufFilePos[64];//路径
    int m_nRow;//行
    int m_nCol;//列,用来判断类型吧
    DWORD m_dwFileDataLen;//文件名字长度
    char m_bufFileDate[1]; //文件名字

};




struct C2SKeybdPkg :public PackageHeader
{
    C2SKeybdPkg() :PackageHeader(C2S_KEY, 0) {}
    C2SKeybdPkg(char cVk)
        :PackageHeader(C2S_KEY, sizeof(*this) - sizeof(PackageHeader)),
        m_cVk(cVk)
    {

    }
    char m_cVk;//按下的键
};

struct S2CFileUploadPkg :public PackageHeader
{
    S2CFileUploadPkg() :
        PackageHeader(S2C_FILE_UPLOAD, 0),
        m_dwFileUploadDataLen(0)
    {}
    S2CFileUploadPkg(LPBYTE pBuff, DWORD dwLen)
        :PackageHeader(S2C_FILE_UPLOAD, sizeof(*this) - sizeof(PackageHeader) + dwLen)
    {
        m_dwFileUploadDataLen = dwLen;
        memcpy(m_bufFileUploadDate, pBuff, m_dwFileUploadDataLen);
    }
    DWORD m_dwFileUploadDataLen;
    char m_bufFileUploadDate[1]; //文件路径

};


struct C2SFileUploadPkg :public PackageHeader
{
    C2SFileUploadPkg() :
        PackageHeader(C2S_FILE_UPLOAD, 0),
        m_dwFileUploadDataLen(0), m_cType(0), m_nSize(0)
    {}
    C2SFileUploadPkg(LPBYTE pBuff, DWORD dwLen, char cType, int nSize)
        :PackageHeader(C2S_FILE_UPLOAD, sizeof(*this) - sizeof(PackageHeader) + dwLen)
    {
        m_cType = cType;
        m_nSize = nSize;
        m_dwFileUploadDataLen = dwLen;
        memcpy(m_bufFileUploadDate, pBuff, m_dwFileUploadDataLen);
    }
    char m_cType;//用来判断是文件数据还是文件大小,1表示文件大小，2表示文件数据
    int m_nSize;//文件的大小
    DWORD m_dwFileUploadDataLen;
    char m_bufFileUploadDate[1]; //文件数据
};

struct C2SFileDownloadPkg :public PackageHeader
{
    C2SFileDownloadPkg() :
        PackageHeader(C2S_FILE_DOWNLOAD, 0),
        m_dwFileDownloadDataLen(0)
    {}
    C2SFileDownloadPkg(LPBYTE pBuff, DWORD dwLen)
        :PackageHeader(C2S_FILE_DOWNLOAD, sizeof(*this) - sizeof(PackageHeader) + dwLen)
    {
        m_dwFileDownloadDataLen = dwLen;
        memcpy(m_bufFileDownloadDate, pBuff, m_dwFileDownloadDataLen);
    }
    DWORD m_dwFileDownloadDataLen;
    char m_bufFileDownloadDate[1]; //文件路径

};


struct S2CFileDownloadPkg :public PackageHeader
{
    S2CFileDownloadPkg() :
        PackageHeader(S2C_FILE_DOWNLOAD, 0),
        m_dwFileDownloadDataLen(0), m_cType(0), m_nSize(0)
    {}
    S2CFileDownloadPkg(LPBYTE pBuff, DWORD dwLen, char cType, int nSize)
        :PackageHeader(S2C_FILE_DOWNLOAD, sizeof(*this) - sizeof(PackageHeader) + dwLen)
    {
        m_cType = cType;
        m_nSize = nSize;
        m_dwFileDownloadDataLen = dwLen;
        memcpy(m_bufFileDownloadDate, pBuff, m_dwFileDownloadDataLen);
    }
    char m_cType;//用来判断是文件数据还是文件大小,1表示文件大小，2表示文件数据
    int m_nSize;//文件的大小
    DWORD m_dwFileDownloadDataLen;
    char m_bufFileDownloadDate[1]; //文件数据

};




struct C2SMousebdPkg :public PackageHeader
{
    C2SMousebdPkg() :PackageHeader(C2S_MOUSE, 0) {}
    C2SMousebdPkg(DWORD dwFlags, int x, int y)
        :PackageHeader(C2S_MOUSE, sizeof(*this) - sizeof(PackageHeader)),
        m_dwFlags(dwFlags), m_x(x), m_y(y)
    {

    }
    int m_x;//当前的横坐标
    int m_y;//当前的纵坐标
    DWORD m_dwFlags;//判断鼠标的类型
};



struct C2SScreenDataPkg :public PackageHeader
{
    C2SScreenDataPkg() :PackageHeader(C2S_SCREENDATA, 0) {}
};


struct C2SCmdPkg :public PackageHeader
{
    C2SCmdPkg() :
        PackageHeader(C2S_CMD, 0),
        m_dwCmdDataLen(0)
    {}
    C2SCmdPkg(LPBYTE pBuff, DWORD dwLen)
        :PackageHeader(C2S_CMD, sizeof(*this) - sizeof(PackageHeader) + dwLen)
    {
        m_dwCmdDataLen = dwLen;
        memcpy(m_bufCmdDate, pBuff, m_dwCmdDataLen);
    }
    DWORD m_dwCmdDataLen;
    char m_bufCmdDate[1]; //需要执行的命令数据

};


struct S2CCmdPkg :public PackageHeader
{
    S2CCmdPkg() :
        PackageHeader(S2C_CMD, 0),
        m_dwCmdDataLen(0)
    {}
    S2CCmdPkg(LPBYTE pBuff, DWORD dwLen)
        :PackageHeader(S2C_CMD, sizeof(*this) - sizeof(PackageHeader) + dwLen)
    {
        m_dwCmdDataLen = dwLen;
        memcpy(m_bufCmdDate, pBuff, m_dwCmdDataLen);
    }
    DWORD m_dwCmdDataLen;
    char m_bufCmdDate[1]; //返回命令数据数据

};


struct S2CScreenDataPkg :public PackageHeader
{
    S2CScreenDataPkg() :
        PackageHeader(S2C_SCREENDATA, 0),
        m_dwWith(0),
        m_dwHeight(0),
        m_dwScreenDataLen(0)
    {}
    S2CScreenDataPkg(DWORD dwWith, DWORD dwHeight, LPBYTE pBuff, DWORD dwLen)
        :PackageHeader(S2C_SCREENDATA, sizeof(*this) - sizeof(PackageHeader) + dwLen),
        m_dwWith(dwWith),
        m_dwHeight(dwHeight)
    {
        m_dwScreenDataLen = dwLen;
        memcpy(m_bufScreenData, pBuff, m_dwScreenDataLen);
    }
    DWORD m_dwWith;//宽度
    DWORD m_dwHeight;//高度
    DWORD m_dwScreenDataLen;
    BYTE m_bufScreenData[1]; //屏幕数据
};

#pragma pack(pop)

//统一的构造包的函数


template<typename T, class... ARGS>
shared_ptr<T> make_package(int nSize, ARGS&&... args)
{
    char* pBuf = new char[nSize];
    return shared_ptr<T>(new (pBuf) T(forward<ARGS>(args)...));
}

/*
返回值：
 >0 - 数据接受成功
 0  - 连接断开
 -1 - 数据接受错误
*/
int RecvPack(SOCKET sock, PackageHeader*& pPkg);
int SendPack(SOCKET sock, PackageHeader* pPkg);
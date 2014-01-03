#pragma once

#include <sdk/shared_ptr.h>
#include <string>
// CBookListDlg 对话框

class CBookListDlg : public CDialog
{
    DECLARE_DYNAMIC( CBookListDlg )

public:
    CBookListDlg( shared_ptr<std::string> pResultAddr, bool NotInsertMulSignAddress = true,
                  CWnd* pParent = NULL );  // 标准构造函数
    virtual ~CBookListDlg();

// 对话框数据
    enum { IDD = IDD_DIALOG_BOOK };
protected:
    shared_ptr<std::string> m_pStr;
    bool m_HideMulSignAddress;
protected:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange( CDataExchange* pDX );  // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
};

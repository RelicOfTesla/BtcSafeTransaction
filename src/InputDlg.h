#pragma once

#include <string>
#include <sdk/shared_ptr.h>

// CInputDlg 对话框

class CInputDlg : public CDialog
{
	DECLARE_DYNAMIC(CInputDlg)

public:
	CInputDlg(shared_ptr<std::string> pStr, const std::string& title, bool bPass, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInputDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_INPUT };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
protected:
	shared_ptr<std::string> m_pStr;
	std::string m_title;
	bool m_ispass;
public:
	afx_msg void OnBnClickedOk();
};

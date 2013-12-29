
// BtcSafeTransactionDlg.h : 头文件
//

#pragma once

#include <sdk/shared_ptr.h>
#include <string>

// CBtcSafeTransactionDlg 对话框
class CBtcSafeTransactionDlg : public CDialog
{
// 构造
public:
	CBtcSafeTransactionDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_BTCSAFETRANSACTION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
protected:
	shared_ptr<std::string> GetInputPassword();
	void InsertLog(const std::string& str);
	void UpdateDisableControl();
public:
	afx_msg void OnClose();
	afx_msg void OnEnChangeEditSendAmount();
	afx_msg void OnBnClickedButtonGetP2psignAddr();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonSendMoneyToP2psigAddr();
	afx_msg void OnBnClickedButtonEditP2psigAddrLabel();
	afx_msg void OnBnClickedButtonSelRecvAddr();
	afx_msg void OnBnClickedButtonRecvFromP2psigAddr();
	afx_msg void OnBnClickedButtonSigEndAndSend();
	afx_msg void OnBnClickedButtonConfig();
	afx_msg void OnBnClickedButtonEditP2psigAddrLabelTempl();
	afx_msg void OnBnClickedRadioModeSend1();
	afx_msg void OnBnClickedRadio1ModeRecv2();
	afx_msg void OnBnClickedRadio1ModeRecv1();
	afx_msg void OnBnClickedRadioModeSend2();
	afx_msg void OnBnClickedButtonWebView();
};

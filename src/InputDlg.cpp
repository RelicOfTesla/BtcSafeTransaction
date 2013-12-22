// src/InputDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BtcSafeTransaction.h"
#include "src/InputDlg.h"
#include "util.h"

//////////////////////////////////////////////////////////////////////////
UINT InputDlg_DoModal(shared_ptr<std::string> pstr, const std::string& title, bool bPass)
{
	CInputDlg dlg(pstr, title, bPass);
	return dlg.DoModal();
}
//////////////////////////////////////////////////////////////////////////

// CInputDlg 对话框

IMPLEMENT_DYNAMIC(CInputDlg, CDialog)

CInputDlg::CInputDlg(shared_ptr<std::string> pStr, const std::string& title, bool bPass, CWnd* pParent /*=NULL*/)
	: CDialog(CInputDlg::IDD, pParent), 
	m_pStr(pStr), m_title(title), m_ispass(bPass)
{
	m_pStr->reserve(256);
}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInputDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CInputDlg::OnBnClickedOk)
END_MESSAGE_MAP()


BOOL CInputDlg::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();

	this->SetWindowText(m_title.c_str());

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_INPUT_VAL);
	if (m_ispass)
	{
		pEdit->SetPasswordChar('*');
		pEdit->ModifyStyle(0, ES_PASSWORD);
	}
	else
	{
		pEdit->SetPasswordChar(0);
		pEdit->ModifyStyle(ES_PASSWORD, 0);
	}

	return ret;
}


void CInputDlg::OnBnClickedOk()
{
	*m_pStr = GetWindowStlText(GetDlgItem(IDC_EDIT_INPUT_VAL));
	m_pStr.reset();
	CDialog::OnOK();
}



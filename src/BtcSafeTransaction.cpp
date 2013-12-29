#include "stdafx.h"
#include "BtcSafeTransaction.h"
#include "BtcSafeTransactionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CBtcSafeTransactionApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()



CBtcSafeTransactionApp::CBtcSafeTransactionApp()
{
}



CBtcSafeTransactionApp theApp;



BOOL CBtcSafeTransactionApp::InitInstance()
{

	CWinApp::InitInstance();
	CoInitializeEx(0,0);

	AfxEnableControlContainer();

#if _MFC_VER >= 0x0A00
	CShellManager *pShellManager = new CShellManager;
#endif


	CBtcSafeTransactionDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

#if _MFC_VER >= 0x0A00
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}
#endif

	return FALSE;
}


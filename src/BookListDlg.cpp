#include "stdafx.h"
#include "BtcSafeTransaction.h"
#include "BookListDlg.h"
#include "rpc_helper.hpp"
#include "util.h"

//////////////////////////////////////////////////////////////////////////
extern shared_ptr<CRpcHelper> g_pRpcHelper;

UINT BookDlg_DoModal( shared_ptr<std::string> pstr, bool HideMulSignAddress )
{
    CBookListDlg dlg( pstr, HideMulSignAddress );
    return dlg.DoModal();
}
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC( CBookListDlg, CDialog )

CBookListDlg::CBookListDlg( shared_ptr<std::string> pStr, bool HideMulSignAddress, CWnd* pParent /*=NULL*/ )
    : CDialog( CBookListDlg::IDD, pParent ),
      m_pStr( pStr ), m_HideMulSignAddress( HideMulSignAddress )
{

}

CBookListDlg::~CBookListDlg()
{
}

void CBookListDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
}

BOOL CBookListDlg::OnInitDialog()
{
    __super::OnInitDialog();

    CListCtrl* pList = ( CListCtrl* )GetDlgItem( IDC_LIST1 );
    assert( pList );

    pList->InsertColumn( 0, "Address", LVCFMT_LEFT, 350 );
    pList->InsertColumn( 1, "Comment", LVCFMT_LEFT, 400 );
    pList->SetExtendedStyle( pList->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

    assert( g_pRpcHelper );

    CRpcHelper::mini_booklist booklist;

    try
    {
        booklist = g_pRpcHelper->GetMiniBookList();
    }
    catch( std::exception& e )
    {
        AfxMessageBox( e.what(), MB_ICONERROR );
    }


    for( auto it = booklist.begin(); it != booklist.end(); ++it )
    {
        if( m_HideMulSignAddress )
        {
            if( IsMulSigAddr( it->first ) )
            {
                continue;
            }
        }
        int nItem = pList->InsertItem( pList->GetItemCount(), "" );
        pList->SetItemText( nItem, 0, it->first.c_str() );
        pList->SetItemText( nItem, 1, it->second.c_str() );
    }
    return TRUE;
}


BEGIN_MESSAGE_MAP( CBookListDlg, CDialog )
    ON_BN_CLICKED( IDOK, &CBookListDlg::OnBnClickedOk )
END_MESSAGE_MAP()


// CBookListDlg 消息处理程序



void CBookListDlg::OnBnClickedOk()
{
    CListCtrl* pList = ( CListCtrl* )GetDlgItem( IDC_LIST1 );
    assert( pList );
    if( POSITION pos = pList->GetFirstSelectedItemPosition() )
    {
        int nItem = pList->GetNextSelectedItem( pos );
        assert( nItem >= 0 );
        CString str = pList->GetItemText( nItem, 0 );

        *m_pStr = str;
    }

    // TODO: 在此添加控件通知处理程序代码
    CDialog::OnOK();
}

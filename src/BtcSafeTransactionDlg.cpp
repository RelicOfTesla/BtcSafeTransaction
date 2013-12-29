#include "stdafx.h"
#include "BtcSafeTransaction.h"
#include "BtcSafeTransactionDlg.h"
#include <time.h>
#include <process.h>

#include <boost/algorithm/string.hpp>
#include <boost/thread/mutex.hpp>
#include <sdk/util/AppPath.hpp>
#include <sdk/str_format.h>

#include "rpc_helper.hpp"
#include "rpc_process.h"
#include "option.h"
#include "util.h"
#include "lang_str.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WALLET_SAVE_NOTFULL_MULSIG_TX	0
#define RPC_CAN_QUERY_MULSIG_BALANCE	WALLET_SAVE_NOTFULL_MULSIG_TX
#define RPC_CAN_TRACE_MULSIG_SPENT		0

#define RPC_CAN_MODIFY_LABEL	0

enum
{
    TimerID_CheckProcess = 1001,
    TimerID_RefreshPubKey = 1002,
    TimerID_RefreshMulSigAddrBalance = 1003,
};

const UINT TimeInterval_CheckRPC_error = 2 * 1000;
const UINT TimeInterval_CheckRPC_ok = 15 * 1000;
const UINT TimeInterval_CheckProcess = 3 * 1000;
const UINT TimeInterval_RefreshPubKey = 20 * 1000;
const UINT TimeInterval_RefreshBalance = 20 * 1000;

const std::string DefaultPubCoinLabel = "BtcSafeTransaction_PubKey";

shared_ptr<CRpcHelper> g_pRpcHelper;
shared_ptr<CAppOption> g_pOption;
char g_CoinAddr_FirstChar = 0;

#define MIN_SEND_AMOUNT 0.0000001
#define ZERO_AMOUNT 0.0000001

//////////////////////////////////////////////////////////////////////////
UINT SettingDlg_DoModal();
UINT BookDlg_DoModal(shared_ptr<std::string> pstr, bool HideMulSignAddress = true);
UINT InputDlg_DoModal(shared_ptr<std::string> pstr, const std::string& title, bool bPass);

void PushError(const std::string& ec);
std::string HttpGet(const std::string& url);
CRpcHelper::unspent_info WebQuery_GetUnspent(const std::string& addr);
double WebQuery_GetMulSignBalance(const std::string& addr);
//////////////////////////////////////////////////////////////////////////


shared_ptr<CCoinJsonRpc> CreateJsonRPC_FromOption(shared_ptr<CAppOption> opt)
{
    shared_ptr<CCoinJsonRpc> pRpc(new CCoinJsonRpc(opt->IP, opt->Port, opt->LoginUser, opt->LoginPass));
    return pRpc;
}

shared_ptr<CRpcHelper> CreateRpcHelper(shared_ptr<CAppOption> opt)
{
    shared_ptr<CCoinJsonRpc> pjson = CreateJsonRPC_FromOption(opt);
    shared_ptr<CRpcHelper> pRpcHelper(new CRpcHelper(pjson));
    return pRpcHelper;
}
//////////////////////////////////////////////////////////////////////////

shared_ptr<CAppOption> LoadOption(const std::string& filepath)
{
    shared_ptr<CAppOption> pOption(new CAppOption);
    Json::Value jv = json_from_file(filepath.c_str());

    std::string CoinName = "Bitcoin";
    pOption->Bin_DIR = JsonOptGet_Str(jv, "Bin_DIR", "%ProgramFiles%\\" + CoinName);
    pOption->DB_DIR = JsonOptGet_Str(jv, "DB_DIR", "%AppData%\\" + CoinName);
    pOption->IP = JsonOptGet_Str(jv, "IP", "127.0.0.1");
    pOption->Port = JsonOptGet_UINT(jv, "Port", 8332);
    pOption->LoginUser = JsonOptGet_Str(jv, "LoginUser", MakeRandomString(16));
    pOption->LoginPass = JsonOptGet_Str(jv, "LoginPass", MakeRandomString(16));
    pOption->StartExtern = JsonOptGet_Str(jv, "StartExtern", "");
    pOption->UseUIApp = JsonOptGet_UINT(jv, "UseUIApp", TRUE);	//////////////////
    pOption->PubAddrLabel = JsonOptGet_Str(jv, "PubAddrLabel", DefaultPubCoinLabel);
    pOption->DonateAuthor = JsonOptGet_UINT(jv, "DonateAuthor", FALSE);

    pOption->FileName_UIApp = JsonOptGet_Str(jv, "FileName_UIApp", CoinName + "-qt.exe");	//////////////////
    pOption->FileName_ConsApp = JsonOptGet_Str(jv, "FileName_ConsApp", CoinName + "d.exe");	//////////////////

    pOption->txfee = JsonOptGet_double(jv, "txfee", 0.00001);
    pOption->MakeRecvMode = JsonOptGet_UINT(jv, "MakeRecvMode", FALSE);

    return pOption;
}
void SaveOption(shared_ptr<CAppOption> pOption, const std::string& filepath)
{
    Json::Value jv ;//= json_from_file(filepath.c_str());
    jv["Bin_DIR"] = pOption->Bin_DIR;
    jv["DB_DIR"] = pOption->DB_DIR;
    jv["IP"] = pOption->IP;
    jv["Port"] = pOption->Port;
    jv["LoginUser"] = pOption->LoginUser;
    jv["LoginPass"] = pOption->LoginPass;
    jv["StartExtern"] = pOption->StartExtern;
    jv["UseUIApp"] = pOption->UseUIApp;	//////////////////
    jv["PubAddrLabel"] = pOption->PubAddrLabel;
    jv["DonateAuthor"] = pOption->DonateAuthor;

    jv["FileName_UIApp"] = pOption->FileName_UIApp; //////////////////
    jv["FileName_ConsApp"] = pOption->FileName_ConsApp; //////////////////

    jv["txfee"] = pOption->txfee;
    jv["MakeRecvMode"] = pOption->MakeRecvMode;

    json_to_file(jv, filepath.c_str());
}

void InitMyAppOption()
{
    g_pOption = LoadOption( GetAppFile("bst_config.json") );;
}
void SaveMyAppOption()
{
    SaveOption(g_pOption, GetAppFile("bst_config.json") );
}
//////////////////////////////////////////////////////////////////////////

CBtcSafeTransactionDlg::CBtcSafeTransactionDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CBtcSafeTransactionDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBtcSafeTransactionDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBtcSafeTransactionDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_GET_P2PSIGN_ADDR, &CBtcSafeTransactionDlg::OnBnClickedButtonGetP2psignAddr)
    ON_BN_CLICKED(IDC_BUTTON_SEND_MONEY_TO_P2PSIG_ADDR, &CBtcSafeTransactionDlg::OnBnClickedButtonSendMoneyToP2psigAddr)
    ON_BN_CLICKED(IDC_BUTTON_EDIT_P2PSIG_ADDR_LABEL, &CBtcSafeTransactionDlg::OnBnClickedButtonEditP2psigAddrLabel)
    ON_BN_CLICKED(IDC_BUTTON_SEL_RECV_ADDR, &CBtcSafeTransactionDlg::OnBnClickedButtonSelRecvAddr)
    ON_BN_CLICKED(IDC_BUTTON_RECV_FROM_P2PSIG_ADDR, &CBtcSafeTransactionDlg::OnBnClickedButtonRecvFromP2psigAddr)
    ON_BN_CLICKED(IDC_BUTTON_SIG_END_AND_SEND, &CBtcSafeTransactionDlg::OnBnClickedButtonSigEndAndSend)
    ON_BN_CLICKED(IDC_BUTTON_CONFIG, &CBtcSafeTransactionDlg::OnBnClickedButtonConfig)
    ON_BN_CLICKED(IDC_BUTTON_EDIT_P2PSIG_ADDR_LABEL_TEMPL, &CBtcSafeTransactionDlg::OnBnClickedButtonEditP2psigAddrLabelTempl)
    ON_BN_CLICKED(IDC_RADIO_MODE_SEND_1, &CBtcSafeTransactionDlg::OnBnClickedRadioModeSend1)
    ON_BN_CLICKED(IDC_RADIO_MODE_RECV_2, &CBtcSafeTransactionDlg::OnBnClickedRadio1ModeRecv2)
    ON_BN_CLICKED(IDC_RADIO_MODE_RECV_1, &CBtcSafeTransactionDlg::OnBnClickedRadio1ModeRecv1)
    ON_BN_CLICKED(IDC_RADIO_MODE_SEND_2, &CBtcSafeTransactionDlg::OnBnClickedRadioModeSend2)
    ON_BN_CLICKED(IDC_BUTTON_WEB_VIEW, &CBtcSafeTransactionDlg::OnBnClickedButtonWebView)
END_MESSAGE_MAP()

bool GetCertCheck(const std::string& filepath);

BOOL CBtcSafeTransactionDlg::OnInitDialog()
{
    srand((UINT)time(0));

    CDialog::OnInitDialog();

#if !_DEBUG
    if (!GetCertCheck(GetAppPath()))
    {
        AfxMessageBox(STR_APP_FALSIFIED);
        PostMessage(WM_CLOSE, 0, 0);
    }
#endif

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    InitMyAppOption();
    g_pRpcHelper = CreateRpcHelper(g_pOption);

    if (g_pOption->MakeRecvMode)
    {
        CButton* pRadio = (CButton*)GetDlgItem(IDC_RADIO_MODE_RECV_1);
        pRadio->SetCheck( TRUE );
    }
    else
    {
        CButton* pRadio = (CButton*)GetDlgItem(IDC_RADIO_MODE_SEND_1);
        pRadio->SetCheck( TRUE );
    }
    UpdateDisableControl();

    InsertLog(STR_APP_SRC_URL);
    InsertLog(STR_AUTHOR_HOME);

    SetTimer(TimerID_CheckProcess, TimeInterval_CheckProcess, 0);
    SetTimer(TimerID_RefreshPubKey, TimeInterval_RefreshPubKey, 0);
    PostMessage(WM_TIMER, TimerID_CheckProcess, 0);
    return TRUE;
}


void CBtcSafeTransactionDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

HCURSOR CBtcSafeTransactionDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CBtcSafeTransactionDlg::OnClose()
{
    SaveMyAppOption();

    CDialog::EndDialog(0);
}
void CBtcSafeTransactionDlg::OnOK()
{

}
void CBtcSafeTransactionDlg::OnCancel()
{

}

struct MakePubKeyInfo
{
    CRpcHelper::pubkey_str PubKey1, PubKey2, PubKey3;
};

MakePubKeyInfo GetMakePubKeyInfo(CWnd* pDlg)
{

    MakePubKeyInfo info;
    info.PubKey1 = CRpcHelper::pubkey_str( GetWindowStlText(pDlg->GetDlgItem(IDC_EDIT_PUBKEY_1)) );
    info.PubKey2 = CRpcHelper::pubkey_str( GetWindowStlText(pDlg->GetDlgItem(IDC_EDIT_PUBKEY_2)) );
    info.PubKey3 = CRpcHelper::pubkey_str( GetWindowStlText(pDlg->GetDlgItem(IDC_EDIT_PUBKEY_3)) );

    boost::algorithm::trim(info.PubKey1);
    boost::algorithm::trim(info.PubKey2);
    boost::algorithm::trim(info.PubKey3);

    if (info.PubKey1.empty())
    {
        throw std::logic_error(STR_PUBKEY1_ERROR);
    }
    if (info.PubKey2.empty())
    {
        throw std::logic_error(STR_PUBKEY2_ERROR);
    }
    if (info.PubKey1 == info.PubKey2 || info.PubKey1 == info.PubKey3 || info.PubKey2 == info.PubKey3)
    {
        throw std::logic_error(STR_SAME_PUBKEYS);
    }
    if ( g_pOption->MakeRecvMode )
    {
        std::swap(info.PubKey1, info.PubKey2);
    }

    return info;

}

void CBtcSafeTransactionDlg::OnBnClickedButtonGetP2psignAddr()
{

    try
    {
        MakePubKeyInfo makeinfo = GetMakePubKeyInfo(this);

        CRpcHelper::address_str MulSigAddr = g_pRpcHelper->NewMulSigAddr(makeinfo.PubKey1, makeinfo.PubKey2, makeinfo.PubKey3);
        SetDlgItemText(IDC_EDIT_P2PSIG_ADDR, MulSigAddr.c_str());

        std::string label = g_pRpcHelper->GetLabel(MulSigAddr);
        if(label.empty())
        {
            if (g_pOption->MakeRecvMode)
            {
                label = STR_DEF_COMMENT_WILL_SELL;
            }
            else
            {
                label = STR_DEF_COMMENT_WILL_BUY;
            }
            g_pRpcHelper->SetMulSigAddrLabel(makeinfo.PubKey1, makeinfo.PubKey2, makeinfo.PubKey3, CRpcHelper::label_str(label));
        }
        SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_COMMENT, label.c_str());

        SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_BALANCE, "");
        PostMessage(WM_TIMER, TimerID_RefreshMulSigAddrBalance, 0);
        SetTimer(TimerID_RefreshMulSigAddrBalance, TimeInterval_RefreshBalance, 0);

    }
    catch(std::exception& e)
    {
        AfxMessageBox(e.what(), MB_ICONERROR);
    }

}

void CBtcSafeTransactionDlg::OnBnClickedButtonSendMoneyToP2psigAddr()
{
    try
    {
        CRpcHelper::address_str p2psig_addr = CRpcHelper::address_str( GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR)) );
        if (p2psig_addr.empty())
        {
            throw std::runtime_error(STR_MULSIG_ADDR_EMPTY);
        }
        std::string szAmount = GetWindowStlText(GetDlgItem(IDC_EDIT_SEND_AMOUNT));
        double fAmount = atof(szAmount.c_str());
        if (fAmount <= MIN_SEND_AMOUNT)
        {
            throw std::runtime_error(STR_SEND_MONEY_SMALL);
        }

        std::string sztip = str_format(STR_ASK_SEND_MONEY_TO_ADDR,
                                       amount2str(fAmount).c_str(), p2psig_addr.c_str()
                                      );
        if (AfxMessageBox(sztip.c_str(), MB_YESNO) == IDNO)
        {
            return;
        }

        std::string txid;
        try
        {
            //g_pRpcHelper->SetTxFee(g_pOption->txfee);
            txid = g_pRpcHelper->SendAmount(p2psig_addr, fAmount);
        }
        catch(const rpc_exception& e)
        {
            bool isok = false;
            if (e.get_error_code() == RPC_WALLET_UNLOCK_NEEDED)
            {
                shared_ptr<std::string> pstr = GetInputPassword();
                if (pstr->size())
                {
                    g_pRpcHelper->EnterPassword(*pstr);
                    txid = g_pRpcHelper->SendAmount(p2psig_addr, fAmount);
                    isok = true;
                }
            }
            if (!isok)
            {
                throw;
            }
        }
        SetDlgItemText(IDC_EDIT_SEND_AMOUNT, "");
        InsertLog(STR_LOG_SENDED_MONEY_TO_MUL_ADDR);
        InsertLog(STR_TIP_PREFIX_TXID + txid);

    }
    catch(std::exception& e)
    {
        AfxMessageBox(e.what(), MB_ICONERROR);
        return;
    }
}

void fill_and_delete_pstring(std::string* ps)
{
    std::fill(ps->begin(), ps->end(), ' ');
    delete ps;
}

shared_ptr<std::string> CBtcSafeTransactionDlg::GetInputPassword()
{
    shared_ptr<std::string> pStr(new std::string, fill_and_delete_pstring);
    pStr->reserve(256);
    InputDlg_DoModal(pStr, STR_TIP_INPUT_PASS, true);
    return pStr;
}

void CBtcSafeTransactionDlg::OnBnClickedButtonEditP2psigAddrLabel()
{
    try
    {
        std::string p2psig_addr = GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR));
        if (p2psig_addr.empty())
        {
            throw std::runtime_error(STR_MULSIG_ADDR_EMPTY);
        }

        std::string comment = GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR_COMMENT));
        boost::algorithm::trim(comment);
#if RPC_CAN_MODIFY_LABEL
        g_pRpcHelper->SetLabel(p2psig_addr, comment);
#else
        MakePubKeyInfo makeinfo = GetMakePubKeyInfo(this);
        g_pRpcHelper->SetMulSigAddrLabel(makeinfo.PubKey1, makeinfo.PubKey2, makeinfo.PubKey3, CRpcHelper::label_str(comment));
#endif
    }
    catch(std::exception& e)
    {
        AfxMessageBox(e.what(), MB_ICONERROR);
        return;
    }

}


void CBtcSafeTransactionDlg::OnBnClickedButtonEditP2psigAddrLabelTempl()
{
    try
    {
        std::string p2psig_addr = GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR));
        if (p2psig_addr.empty())
        {
            throw std::runtime_error(STR_MULSIG_ADDR_EMPTY);
        }


        std::string UiPubKey2 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_2));
        std::string UiPubKey3 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_3));

        boost::algorithm::trim(UiPubKey2);
        boost::algorithm::trim(UiPubKey3);

        std::string comment = UiPubKey2 + "/" + UiPubKey3;
        if (g_pOption->PubAddrLabel != DefaultPubCoinLabel)
        {
            if (AfxMessageBox(STR_ASK_INSERT_MY_PUBKEY_TO_COMMENT, MB_YESNO) == IDYES)
            {
                std::string UiPubKey1 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_1));
                boost::algorithm::trim(UiPubKey1);
                comment += "/" + UiPubKey1;
            }
        }
#if RPC_CAN_MODIFY_LABEL
        g_pRpcHelper->SetLabel(p2psig_addr, comment);
#else
        MakePubKeyInfo makeinfo = GetMakePubKeyInfo(this);
        g_pRpcHelper->SetMulSigAddrLabel(makeinfo.PubKey1, makeinfo.PubKey2, makeinfo.PubKey3, CRpcHelper::label_str(comment));
#endif
        SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_COMMENT, comment.c_str());
    }
    catch(std::exception& e)
    {
        AfxMessageBox(e.what(), MB_ICONERROR);
        return;
    }
}


void CBtcSafeTransactionDlg::OnBnClickedButtonSelRecvAddr()
{
    shared_ptr<std::string> pstr(new std::string);
    BookDlg_DoModal(pstr);
    if( pstr->size() )
    {
        GetDlgItem(IDC_EDIT_RECV_ADDR)->SetWindowText(pstr->c_str());
    }
}

size_t search_vout(const CRpcHelper::TxDataInfo& txinfo, const std::string& addr)
{
    for (size_t i = 0; i < txinfo.dest_list.size(); ++i)
    {
        const CRpcHelper::TxDataInfo::TxDestInfo& r = txinfo.dest_list[i];
        if( r.addr.count(addr) > 0 )
        {
            return i;
        }
    }
    throw std::runtime_error(STR_CAN_NOT_FIND_TX_VOUT);
}

void RemoveSpentTransaction(CRpcHelper::unspent_info&)
{
#if RPC_CAN_TRACE_MULSIG_SPENT
#else
    printf("warning: can't get mulsig spent transaction");
    // not support, must make saved database.
    // but, when i'm send out, bitcoin was checked tx spented.
#endif
}

CRpcHelper::unspent_info GetUnspentData_FromMulSigAddr(const CRpcHelper::address_str& addr)
{
    CRpcHelper::unspent_info unspent = g_pRpcHelper->GetUnspentData_FromRecvAddr(addr);
#if WALLET_SAVE_NOTFULL_MULSIG_TX
    assert(unspent.txlist.size());
    return unspent;
#else
    if (unspent.txlist.empty())
    {
        unspent = WebQuery_GetUnspent(addr);
    }

    if (unspent.txlist.empty())
    {
        shared_ptr<std::string> pTxID(new std::string);
        InputDlg_DoModal(pTxID, STR_UNSPENT_TX_INPUT_TITLE, false);
        if (pTxID->empty())
        {
            throw std::logic_error(STR_TX_INPUT_EMPTY);
        }
        std::list<std::string> strlist;
        // #DEFINE _SCL_SECURE_NO_WARNINGS
        boost::split(strlist, *pTxID, boost::is_any_of(",-;/| "));
        for (std::list<std::string>::iterator it = strlist.begin(); it != strlist.end(); ++it)
        {
            CRpcHelper::unk_txfrom_info from;
            from.txid = CRpcHelper::txid_str( boost::algorithm::trim_copy(*it) );
            if (from.txid.empty())
            {
                continue;
            }
            CRpcHelper::TxDataInfo query_tx = g_pRpcHelper->GetTransactionInfo_FromTxId( from.txid );
            from.vout = search_vout(query_tx, addr);
            // verify(from.vout>=0);
            unspent.total += query_tx.dest_list[from.vout].value;
            unspent.txlist.push_back(from);
        }
        RemoveSpentTransaction(unspent);
		if (unspent.txlist.empty())
		{
			throw std::logic_error(STR_UNSPENT_TX_EMPTY);
		}
    }
#endif
    return unspent;
}


void CBtcSafeTransactionDlg::OnBnClickedButtonRecvFromP2psigAddr()
{
    try
    {

        CRpcHelper::address_str p2psig_addr = CRpcHelper::address_str( GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR)) );
        if (p2psig_addr.empty())
        {
            throw std::logic_error(STR_MULSIG_ADDR_EMPTY);
        }
        CRpcHelper::address_str szRecvAddr = CRpcHelper::address_str( GetWindowStlText(GetDlgItem(IDC_EDIT_RECV_ADDR)) );
        boost::algorithm::trim(szRecvAddr);
        if (szRecvAddr.empty())
        {
            throw std::logic_error(STR_REQUEST_RECV_ADDR_EMPTY);
        }

        CRpcHelper::unspent_info unspent = GetUnspentData_FromMulSigAddr(p2psig_addr);
        if (unspent.txlist.empty())
        {
            throw std::runtime_error(STR_UNSPENT_TX_EMPTY);
        }

        double fAmount = atof( GetWindowStlText(GetDlgItem(IDC_EDIT_SEND_AMOUNT)).c_str() );
        double txfee = g_pOption->txfee;
        if (fAmount + txfee > unspent.total)
        {
            fAmount = unspent.total - txfee;
        }
        if ( fAmount <= MIN_SEND_AMOUNT )
        {
            throw std::logic_error(STR_RECV_REQUEST_AMOUNT_SMALL);
        }
        if (atof(GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR_BALANCE)).c_str()) <= ZERO_AMOUNT)
        {
            SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_BALANCE, amount2str(unspent.total).c_str());
        }
        if ( fabs(unspent.total - fAmount - txfee ) > ZERO_AMOUNT )
        {
            std::string sztip = str_format(STR_TIP_RECV_AMOUNT_NOT_EQUAL_BALANCE,
                                           amount2str(fAmount).c_str(),
                                           amount2str(unspent.total).c_str()
                                          );
            if( AfxMessageBox(sztip.c_str(), MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO )
            {
                return;
            }
        }
        // ------
        CRpcHelper::payout_list paylist;
        CRpcHelper::payout_record pay;
        pay.addr = szRecvAddr;
        pay.fAmount = fAmount;
        paylist.push_back(pay);

        pay.addr = p2psig_addr;
        pay.fAmount = unspent.total - (pay.fAmount + txfee);
        if (pay.fAmount > 0)
        {
            paylist.push_back(pay); // send back.

            assert( paylist.begin()->fAmount + (++paylist.begin())->fAmount + txfee == unspent.total );
        }
        else
        {
            assert( paylist.begin()->fAmount + txfee == unspent.total );
        }

        if (g_pOption->DonateAuthor) // default FALSE.
        {
			// 捐赠0.05%(万分之5)给作者[可选开关,默认不捐赠]
			
            if (g_CoinAddr_FirstChar == '1')
            {
                pay.addr.assign("161WBgKjso7Yht57NgVgnSEkvVui3mcpe4");
            }
            else if (g_CoinAddr_FirstChar == 'L')
            {
                pay.addr.assign("LfnLtw1My7odaaNnPaE3Sur9p1dWGUTsEd");
            }
            else
            {
                pay.addr.clear();
            }

            if (pay.addr.size())
            {
                pay.fAmount = 0;
                const double rate = 0.05f / 100;
                for (CRpcHelper::payout_list::iterator it = paylist.begin(); it != paylist.end(); ++it)
                {
                    CRpcHelper::payout_record& rv = *it;
                    double sub = rv.fAmount * rate;
                    pay.fAmount += sub;
                    rv.fAmount -= sub;
                }
                if (pay.fAmount > MIN_SEND_AMOUNT)
                {
                    paylist.push_back(pay);
                }
            }
        }
        CRpcHelper::txdata_str txdata = g_pRpcHelper->CreateRawTransaction(unspent.txlist, paylist);
        try
        {
            txdata = g_pRpcHelper->SignRawTransaction(txdata);
        }
        catch(const rpc_exception& e)
        {
            bool isok = false;
            if (e.get_error_code() == RPC_WALLET_UNLOCK_NEEDED)
            {
                shared_ptr<std::string> pstr = GetInputPassword();
                if (pstr->size())
                {
                    g_pRpcHelper->EnterPassword(*pstr);
                    txdata = g_pRpcHelper->SignRawTransaction(txdata);
                    isok = true;
                }
            }
            if (!isok)
            {
                throw;
            }
        }

        SetDlgItemText(IDC_EDIT_TX_DATA, txdata.c_str());
        InsertLog(STR_TX_REQUEST_BUILDED);
    }
    catch(std::exception& e)
    {
        AfxMessageBox(e.what(), MB_ICONERROR);
        return;
    }
}


bool VerifyTxData(const CRpcHelper::txdata_str& txdata)
{
    if (txdata.empty())
    {
        throw std::logic_error(STR_ERROR_TX_DATA_EMPTY);
    }
    CRpcHelper::TxDataInfo txinfo = g_pRpcHelper->GetTransactionInfo_FromData(txdata);
    if (txinfo.src_txid_list.empty())
    {
        throw std::runtime_error(STR_ERROR_TX_DATA_WITH_SRCTX_EMPTY);
    }
    double to_amount = 0;
    for (auto it = txinfo.dest_list.begin(); it != txinfo.dest_list.end(); ++it)
    {
        const CRpcHelper::TxDataInfo::TxDestInfo& info = *it;
        to_amount += info.value;
    }

    std::set<std::string> from_addr_list;
    double from_balance = 0;
    for (auto it_s = txinfo.src_txid_list.begin(); it_s != txinfo.src_txid_list.end(); ++it_s)
    {
        CRpcHelper::unk_txfrom_info& rv = *it_s;
        CRpcHelper::TxDataInfo src = g_pRpcHelper->GetTransactionInfo_FromTxId( rv.txid );
        if (rv.vout >= 0 && rv.vout < src.dest_list.size())
        {
            const CRpcHelper::TxDataInfo::TxDestInfo& info = src.dest_list[rv.vout];
            from_addr_list.insert( info.addr.begin(), info.addr.end() );
            from_balance += info.value;
        }
    }
    if ( fabs(from_balance - to_amount - g_pOption->txfee) > ZERO_AMOUNT )
    {
        std::string sztip = str_format(STR_TIP_BALANCE_NOT_EQUAL_FINAL_AMOUNT,
                                       amount2str(from_balance).c_str(),
                                       amount2str(to_amount).c_str()
                                      );
        PushError(sztip);
        AfxMessageBox(sztip.c_str(), MB_ICONEXCLAMATION);
    }

    std::string except_str;
    std::string from_addr_str;
    for (auto it = from_addr_list.begin(); it != from_addr_list.end(); ++it)
    {
        const std::string& s = *it;
        from_addr_str += s + "\r\n";
        if ( !IsMulSigAddr(s) )
        {
            except_str += s + "\r\n";
        }
    }
    if (except_str.size())
    {
        std::string sztip = str_format(STR_TIP_FIND_EXCEPTION_SEND_FROM, except_str.c_str());
        if (AfxMessageBox(sztip.c_str(), MB_ICONHAND | MB_YESNO | MB_DEFBUTTON2) == IDNO)
        {
            return false;
        }
    }
    std::string sztip = str_format(STR_ASK_SEND_FROM_MUL_ADDR_OF_AMOUNT, from_addr_str.c_str(), amount2str(to_amount).c_str());
    if (AfxMessageBox(sztip.c_str(), MB_YESNO | MB_DEFBUTTON2) == IDNO)
    {
        return false;
    }
    return true;
}

void CBtcSafeTransactionDlg::OnBnClickedButtonSigEndAndSend()
{
    try
    {
        CRpcHelper::txdata_str txdata = CRpcHelper::txdata_str( GetWindowStlText(GetDlgItem(IDC_EDIT_TX_DATA)) );
        boost::algorithm::trim(txdata);

        if (!VerifyTxData(txdata))
        {
            return;
        }
        try
        {
            txdata = g_pRpcHelper->SignRawTransaction(txdata);
        }
        catch(const rpc_exception& e)
        {
            bool isok = false;
            if (e.get_error_code() == RPC_WALLET_UNLOCK_NEEDED)
            {
                shared_ptr<std::string> pstr = GetInputPassword();
                if (pstr->size())
                {
                    g_pRpcHelper->EnterPassword(*pstr);
                    txdata = g_pRpcHelper->SignRawTransaction(txdata);
                    isok = true;
                }
            }
            if (!isok)
            {
                throw;
            }
        }
        g_pRpcHelper->SendRawTransaction(txdata);
        InsertLog(STR_LOG_SENDED_MONEY_FROM_MUL_ADDR);
    }
    catch(std::exception& e)
    {
        AfxMessageBox(e.what(), MB_ICONERROR);
        return;
    }
}

void CBtcSafeTransactionDlg::OnBnClickedButtonConfig()
{
    SettingDlg_DoModal();
    g_pRpcHelper = CreateRpcHelper(g_pOption);
}

void CBtcSafeTransactionDlg::InsertLog(const std::string& str)
{
    std::string sztip = GetWindowStlText(GetDlgItem(IDC_EDIT_LOGTIP));
    sztip = str + "\r\n" + sztip;
    SetDlgItemText(IDC_EDIT_LOGTIP, sztip.c_str());

}
//////////////////////////////////////////////////////////////////////////
class CUniqueStringList
{
public:
    void PushString(const std::string& str)
    {
        locker_type lock(m_mutex);
        if (m_last != str)
        {
            m_list.push_back(str);
            m_last = str;
        }
    }

    std::string PopString()
    {
        locker_type lock(m_mutex);
        while (m_list.size())
        {
            std::string str = m_list.front();
            m_list.pop_front();
            if (str != m_last)
            {
                return str;
            }
        }
        return std::string();
    }

protected:
    std::list<std::string> m_list;
    std::string m_last;
    boost::mutex m_mutex;
    typedef boost::unique_lock<boost::mutex> locker_type;
};

CUniqueStringList g_errorlist;
void PushError(const std::string& ec)
{
    g_errorlist.PushString(ec);
}

void thread_rpc_check(void* rp)
{
    CoInitializeEx(0, 0);
    bool* p = (bool*)rp;
    while (true)
    {
        {
            if( shared_ptr<CRpcHelper> pRpc = g_pRpcHelper )
            {
                *p = g_pRpcHelper->IsRpcCanConnected();
            }
        }
        if (*p)
        {
            Sleep(TimeInterval_CheckRPC_ok);
        }
        else
        {
            Sleep(TimeInterval_CheckRPC_error);
        }
    }
}

bool g_rpc_watch_isinit = false;
bool g_rpc_can_connected = false;
bool Async_IsRpcCanConnected()
{
    if (!g_rpc_watch_isinit)
    {
        g_rpc_watch_isinit = true;
        _beginthread(thread_rpc_check, 0, &g_rpc_can_connected);
        Sleep(500);
    }
    return g_rpc_can_connected;
}

double WebQuery_GetBtcBalance_blockchain_info(const std::string& addr)
{
    bool must_confirmation = false;
    std::string explorer = "https://blockchain.info/q/addressbalance/" + addr;
    if(must_confirmation)
    {
        explorer += "?confirmations=1";
    }
    std::string str = HttpGet(explorer + addr);
    if (str.empty())
    {
        throw std::runtime_error("http query empty");
    }
    return double(atof(str.c_str())) / 100000000;
}
double WebQuery_GetLtcBalance_ltcBlockExplorer(const std::string& addr)
{
    std::string explorer = "http://ltc.block-explorer.com/csv/address/";
    std::string str = HttpGet(explorer + addr);
    if (str.empty())
    {
        throw std::runtime_error("http query empty");
    }
    if (str.substr(0, 3) != "Tx,")
    {
        throw std::runtime_error("invalid query 1");
    }
    size_t rp = str.rfind(',');
    if (rp == std::string::npos)
    {
        throw std::runtime_error("invalid query 2");
    }
    str = str.substr(rp + 1);
    return atof(str.c_str());
}


double WebQuery_GetMulSignBalance(const std::string& addr)
{
    if (g_CoinAddr_FirstChar == '1')
    {
        return WebQuery_GetBtcBalance_blockchain_info(addr);
    }
    else if (g_CoinAddr_FirstChar == 'L')
    {
        return WebQuery_GetLtcBalance_ltcBlockExplorer(addr);
    }
    else
    {
        throw std::runtime_error(STR_TIP_CAN_NOT_WEB_QUERY_BALANCE);
    }
}

CRpcHelper::unspent_info WebQuery_GetUnspent(const std::string& addr)
{
    // not support
    return CRpcHelper::unspent_info();
}


void CBtcSafeTransactionDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    try
    {
        if (nIDEvent == TimerID_CheckProcess)
        {
            try
            {
                if ( Async_IsRpcCanConnected() )
                {
                    SetDlgItemText(IDC_STATIC_PORT_STATE, STR_STATE_CONNECTED);

                    if (GetDlgItem(IDC_EDIT_PUBKEY_1)->GetWindowTextLength() == 0)
                    {
                        CRpcHelper::address_str recvaddr = g_pRpcHelper->GetOrNewAccountAddress( CRpcHelper::label_str(g_pOption->PubAddrLabel) );
                        if (recvaddr.size())
                        {
                            g_CoinAddr_FirstChar = recvaddr[0];
                        }
                        else
                        {
                            g_CoinAddr_FirstChar = 0;
                        }
                        CRpcHelper::pubkey_str PubKey1 = g_pRpcHelper->GetPubKey(recvaddr);
                        assert( PubKey1.size() );
                        SetDlgItemText(IDC_EDIT_PUBKEY_1, PubKey1.c_str());

                    }
                }
                else if ( !HasCoinProcess(g_pOption) )
                {
                    CButton* pIsAutoStart = (CButton*)GetDlgItem(IDC_CHECK_IS_AUTO_START);
                    if ( pIsAutoStart->GetCheck() )
                    {
                        SetDlgItemText(IDC_STATIC_PORT_STATE, STR_STATE_STARTING);
                        StartProcess_FromOption(g_pOption);
                        SetDlgItemText(IDC_EDIT_PUBKEY_1, "");
                    }
                    else
                    {
                        SetDlgItemText(IDC_STATIC_PORT_STATE, STR_STATE_WAIT);
                    }
                }
            }
            catch(std::exception& e)
            {
                PushError(e.what());
            }

            std::string ec = g_errorlist.PopString();
            if (ec.size())
            {
                InsertLog(STR_PREFIX_ERROR + ec);
            }
            return;
        }
        else if (nIDEvent == TimerID_RefreshPubKey)
        {
            if ( Async_IsRpcCanConnected() )
            {
                std::string RealPubKey1 = g_pRpcHelper->GetPubKey(
                                              g_pRpcHelper->GetOrNewAccountAddress( CRpcHelper::label_str(g_pOption->PubAddrLabel) ));
                std::string ShowPubKey1 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_1));
                if (RealPubKey1 != ShowPubKey1)
                {
                    SetDlgItemText(IDC_EDIT_P2PSIG_ADDR, "");
                    SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_BALANCE, "");
                    SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_COMMENT, "");
                    KillTimer(TimerID_RefreshMulSigAddrBalance);
                    g_CoinAddr_FirstChar = 0;
                }
            }

            return;
        }
        else if (nIDEvent == TimerID_RefreshMulSigAddrBalance)
        {
            try
            {
                if(Async_IsRpcCanConnected())
                {
                    std::string ShowMulSigAddr = GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR));
                    if (ShowMulSigAddr.size())
                    {
#if WALLET_SAVE_NOTFULL_MULSIG_TX
                        double balance = g_pRpcHelper->GetBalance_FromRecvAddr(ShowMulSigAddr);
#else
                        double balance = WebQuery_GetMulSignBalance(ShowMulSigAddr);
#endif
                        std::string szBalance = amount2str(balance);
                        if ( GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR_BALANCE)) != szBalance )
                        {
                            SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_BALANCE, szBalance.c_str());
                        }
                    }
                }
            }
            catch(const std::exception&)
            {
                KillTimer(nIDEvent);
                throw;
            }
            return;
        }
    }
    catch(std::exception& e)
    {
        AfxMessageBox(e.what(), MB_ICONERROR);
        return;
    }

    CDialog::OnTimer(nIDEvent);
}

void CBtcSafeTransactionDlg::UpdateDisableControl()
{
    BOOL MakeRecvMode = g_pOption->MakeRecvMode;
    BOOL MustSend = ((CButton*)GetDlgItem(IDC_RADIO_MODE_SEND_1))->GetCheck();

    GetDlgItem(IDC_BUTTON_SEND_MONEY_TO_P2PSIG_ADDR)->EnableWindow(MustSend);
    GetDlgItem(IDC_BUTTON_RECV_FROM_P2PSIG_ADDR)->EnableWindow(MakeRecvMode);
    GetDlgItem(IDC_BUTTON_SEL_RECV_ADDR)->EnableWindow(MakeRecvMode);
    GetDlgItem(IDC_BUTTON_SIG_END_AND_SEND)->EnableWindow( !MakeRecvMode );
}


void CBtcSafeTransactionDlg::OnBnClickedRadioModeSend1()
{
    g_pOption->MakeRecvMode = FALSE;
    UpdateDisableControl();
    SaveMyAppOption();
}


void CBtcSafeTransactionDlg::OnBnClickedRadio1ModeRecv2()
{
    g_pOption->MakeRecvMode = TRUE;
    UpdateDisableControl();
    SaveMyAppOption();
}


void CBtcSafeTransactionDlg::OnBnClickedRadio1ModeRecv1()
{
    g_pOption->MakeRecvMode = TRUE;
    UpdateDisableControl();
    SaveMyAppOption();
}


void CBtcSafeTransactionDlg::OnBnClickedRadioModeSend2()
{
    g_pOption->MakeRecvMode = FALSE;
    UpdateDisableControl();
    SaveMyAppOption();
}


void CBtcSafeTransactionDlg::OnBnClickedButtonWebView()
{
    std::string addr = GetWindowStlText( GetDlgItem(IDC_EDIT_P2PSIG_ADDR) );
    if (addr.size())
    {
        if(g_CoinAddr_FirstChar == '1')
        {
            addr = "https://blockchain.info/address/" + addr;
        }
        else if (g_CoinAddr_FirstChar == 'L')
        {
            addr = "http://ltc.block-explorer.com/address/" + addr;
        }
        else
        {
            return;
        }

        ShellExecute(0, "open", addr.c_str(), NULL, NULL, SW_SHOW);
    }
}

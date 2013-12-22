
// BtcSafeTransactionDlg.cpp : 实现文件
//

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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WALLET_SAVE_NOTFULL_MULSIG_TX 1

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

#define MIN_SEND_AMOUNT 0.0000001
#define ZERO_AMOUNT 0.0000001
//////////////////////////////////////////////////////////////////////////
UINT SettingDlg_DoModal();
UINT BookDlg_DoModal(shared_ptr<std::string> pstr, bool HideMulSignAddress = true);
UINT InputDlg_DoModal(shared_ptr<std::string> pstr, const std::string& title, bool bPass);

void PushError(const std::string& ec);
std::string HttpGet(const std::string& url);
CRpcHelper::txfrom_list WebQuery_GetTxFrom(const std::string& addr);
double WebQuery_GetMulSignBalance(const std::string& addr);
char g_CoinAddr_FirstChar = 0;
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
    pOption->StartExtern = JsonOptGet_Str(jv, "StartExtern", " ");
    pOption->UseUIApp = JsonOptGet_UINT(jv, "UseUIApp", TRUE);	//////////////////
    pOption->PubAddrLabel = JsonOptGet_Str(jv, "PubAddrLabel", DefaultPubCoinLabel);
    pOption->DonateAuthor = JsonOptGet_UINT(jv, "DonateAuthor", FALSE);

    pOption->FileName_UIApp = JsonOptGet_Str(jv, "FileName_UIApp", CoinName + "-qt.exe");	//////////////////
    pOption->FileName_ConsApp = JsonOptGet_Str(jv, "FileName_ConsApp", CoinName + "d.exe");	//////////////////

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
		AfxMessageBox("程序被篡改，禁止使用!");
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

CBtcSafeTransactionDlg::MakePubKeyInfo CBtcSafeTransactionDlg::GetMakePubKeyInfo()
{
    MakePubKeyInfo info;
    info.PubKey1 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_1));
    info.PubKey2 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_2));
    info.PubKey3 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_3));

    boost::algorithm::trim(info.PubKey1);
    boost::algorithm::trim(info.PubKey2);
    boost::algorithm::trim(info.PubKey3);

    if (info.PubKey1.empty())
    {
        throw std::logic_error("公钥1错误");
    }
    if (info.PubKey2.empty())
    {
        throw std::logic_error("公钥2错误");
    }
    if (info.PubKey1 == info.PubKey2 || info.PubKey1 == info.PubKey3 || info.PubKey2 == info.PubKey3)
    {
        throw std::logic_error("存在相同的公钥");
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
        MakePubKeyInfo makeinfo = GetMakePubKeyInfo();

        std::string MulSigAddr = g_pRpcHelper->NewMulSigAddr(makeinfo.PubKey1, makeinfo.PubKey2, makeinfo.PubKey3);
        SetDlgItemText(IDC_EDIT_P2PSIG_ADDR, MulSigAddr.c_str());;

        SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_BALANCE, "");
		PostMessage(WM_TIMER, TimerID_RefreshMulSigAddrBalance, 0);


        std::string label = g_pRpcHelper->GetLabel(MulSigAddr);
        SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_COMMENT, label.c_str());

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
        std::string p2psig_addr = GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR));
        if (p2psig_addr.empty())
        {
            throw std::runtime_error("担保地址为空");
        }
        std::string szAmount = GetWindowStlText(GetDlgItem(IDC_EDIT_SEND_AMOUNT));
        double fAmount = atof(szAmount.c_str());
        if (fAmount <= MIN_SEND_AMOUNT)
        {
            throw std::runtime_error("发送金额过小");
        }

        std::string sztip = str_format("确实要从你钱包转[%s]给[%s]么", amount2str(fAmount).c_str(), p2psig_addr.c_str());
        if (AfxMessageBox(sztip.c_str(), MB_YESNO) == IDNO)
        {
            return;
        }
        try
        {
            g_pRpcHelper->SendAmount(p2psig_addr, fAmount);
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
                    g_pRpcHelper->SendAmount(p2psig_addr, fAmount);
                    isok = true;
                }
            }
            if (!isok)
            {
                throw;
            }
        }
        SetDlgItemText(IDC_EDIT_SEND_AMOUNT, "");
        InsertLog("发送金额至担保地址成功，请发送你的公钥给商家.");

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
    InputDlg_DoModal(pStr, "请输入密码", true);
    return pStr;
}

void CBtcSafeTransactionDlg::OnBnClickedButtonEditP2psigAddrLabel()
{
    try
    {
        std::string p2psig_addr = GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR));
        if (p2psig_addr.empty())
        {
            throw std::runtime_error("担保地址为空");
        }

        std::string comment = GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR_COMMENT));
#if 1
        {
            boost::algorithm::trim(comment);

            MakePubKeyInfo makeinfo = GetMakePubKeyInfo();

            g_pRpcHelper->SetMulSigAddrLabel(makeinfo.PubKey1, makeinfo.PubKey2, makeinfo.PubKey3, comment);
        }
#else
        //g_pRpcHelper->SetLabel(p2psig_addr, comment);
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
            throw std::runtime_error("担保地址为空");
        }


        std::string UiPubKey2 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_2));
        std::string UiPubKey3 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_3));

        boost::algorithm::trim(UiPubKey2);
        boost::algorithm::trim(UiPubKey3);

        std::string comment = UiPubKey2 + "/" + UiPubKey3;
        if (g_pOption->PubAddrLabel != DefaultPubCoinLabel)
        {
            if (AfxMessageBox("是否加自己的公钥也加入备注中", MB_YESNO) == IDYES)
            {
                std::string UiPubKey1 = GetWindowStlText(GetDlgItem(IDC_EDIT_PUBKEY_1));
                boost::algorithm::trim(UiPubKey1);
                comment += "/" + UiPubKey1;
            }
        }
#if 1
        MakePubKeyInfo makeinfo = GetMakePubKeyInfo();
        g_pRpcHelper->SetMulSigAddrLabel(makeinfo.PubKey1, makeinfo.PubKey2, makeinfo.PubKey3, comment);
#else
        //g_pRpcHelper->SetLabel(p2psig_addr, comment);
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

size_t getvout(const CRpcHelper::TxDataInfo& txinfo, const std::string& addr)
{
	for (size_t i = 0; i < txinfo.dest_list.size(); ++i)
	{
		const CRpcHelper::TxDataInfo::TxDestInfo& r = txinfo.dest_list[i];
		if( r.addr.count(addr) > 0 )
		{
			return i;
		}
	}
	throw std::runtime_error("无法读取vout，找不到相关数据，请确认你的数据是最新的/请确认担保地址、买家/商家模式是对的");
}

void CBtcSafeTransactionDlg::OnBnClickedButtonRecvFromP2psigAddr()
{
    try
    {
        std::string p2psig_addr = GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR));
        if (p2psig_addr.empty())
        {
            throw std::logic_error("担保地址为空");
        }
        std::string szRecvAddr = GetWindowStlText(GetDlgItem(IDC_EDIT_RECV_ADDR));
        boost::algorithm::trim(szRecvAddr);
        if (szRecvAddr.empty())
        {
            throw std::logic_error("收款地址错误");
        }

        CRpcHelper::txfrom_list txfrom_list = g_pRpcHelper->GetRecvTransactionList(p2psig_addr);
        if (txfrom_list.empty())
        {
            txfrom_list = WebQuery_GetTxFrom(p2psig_addr);
        }
		std::string balance_type = "余额";
		double balance = atof(GetWindowStlText(GetDlgItem(IDC_EDIT_P2PSIG_ADDR_BALANCE)).c_str());

        if (txfrom_list.empty())
        {
            shared_ptr<std::string> pTxID(new std::string);
            InputDlg_DoModal(pTxID, "未找到最新交易数据，请手工输入[交易ID,交易ID]", false);
            if (pTxID->empty())
            {
                throw std::logic_error("未找到交易数据，请更新blockchain");
            }
			double recv_history = 0;
            std::list<std::string> strlist;
            // #DEFINE _SCL_SECURE_NO_WARNINGS
            boost::split(strlist, *pTxID, boost::is_any_of(",-;/|"));
            for (std::list<std::string>::iterator it = strlist.begin(); it != strlist.end(); ++it)
            {
                CRpcHelper::txfrom_info from;
                from.txid = boost::algorithm::trim_copy(*it);
                CRpcHelper::TxDataInfo query_tx = g_pRpcHelper->GetTransactionInfo_FromData(
                                                      g_pRpcHelper->GetRawTransaction_FromTxId(from.txid) );
                from.vout = getvout(query_tx, p2psig_addr);
                // verify(from.vout>=0);
#if !WALLET_SAVE_NOTFULL_MULSIG_TX
                if (from.vout >= 0 && from.vout < query_tx.dest_list.size())
                {
                    recv_history += query_tx.dest_list[from.vout].value;
                }
                else
                {
                    assert(0);
                }
#endif
                txfrom_list.push_back(from);
            }
#if !WALLET_SAVE_NOTFULL_MULSIG_TX
			balance_type = "历史金额";
			balance = recv_history;
#endif
        }
        else
        {
#if !WALLET_SAVE_NOTFULL_MULSIG_TX
			double recv_history = 0;
            for (CRpcHelper::txfrom_list::iterator it = txfrom_list.begin(); it != txfrom_list.end(); ++it)
            {
                CRpcHelper::TxDataInfo query_tx = g_pRpcHelper->GetTransactionInfo_FromData(
                                                      g_pRpcHelper->GetRawTransaction_FromTxId(from.txid) );
                if (it->vout >= 0 && it->vout < query_tx.dest_list.size())
                {
                    recv_history += query_tx.dest_list[it->vout].value
                }
                else
                {
                    assert(0);
                }
            }
			balance_type = "历史金额";
			balance = recv_history;
#endif
        }
        if (txfrom_list.empty())
        {
            throw std::runtime_error("找不到相关数据，请更新数据库");
        }
#if !WALLET_SAVE_NOTFULL_MULSIG_TX
		SetDlgItemText(IDC_EDIT_P2PSIG_ADDR_BALANCE, amount2str(balance).c_str());
#endif

        std::string szAmount = GetWindowStlText(GetDlgItem(IDC_EDIT_SEND_AMOUNT));
		double fAmount = atof(szAmount.c_str());
        if ( fAmount <= MIN_SEND_AMOUNT )
        {
            throw std::logic_error("收款金额过小");
        }
        if ( fabs(fAmount - balance) > ZERO_AMOUNT )
        {
            std::string sztip = str_format("你要收款的金额(%s)与担保地址%s(%s)不相同，确定继续收款么？",
                                           szAmount.c_str(),
										   balance_type.c_str(),
                                           amount2str(balance).c_str()
                                          );
            if( AfxMessageBox(sztip.c_str(), MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO )
            {
                return;
            }
        }

        CRpcHelper::payout_list paylist;
        CRpcHelper::payout_record pay;
        pay.addr = szRecvAddr;
        pay.fAmount = atof(szAmount.c_str());
        paylist.push_back(pay);

        if (g_pOption->DonateAuthor) // 捐赠0.05%(万分之5)给作者[可选开关,默认不捐赠]
        {
            if (g_CoinAddr_FirstChar == '1')
            {
                pay.addr = "161WBgKjso7Yht57NgVgnSEkvVui3mcpe4";
            }
            else if (g_CoinAddr_FirstChar == 'L')
            {
                pay.addr = "LfnLtw1My7odaaNnPaE3Sur9p1dWGUTsEd";
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
        std::string txdata = g_pRpcHelper->CreateRawTransaction(txfrom_list, paylist);
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
        InsertLog("交易(请求)数据已生成，请发送数据给对方，等待其确认转币.");
    }
    catch(std::exception& e)
    {
        AfxMessageBox(e.what(), MB_ICONERROR);
        return;
    }
}


bool VerifyTxData(const std::string& txdata)
{
    if (txdata.empty())
    {
        throw std::logic_error("交易数据为空");
    }
    CRpcHelper::TxDataInfo txinfo = g_pRpcHelper->GetTransactionInfo_FromData(txdata);
    if (txinfo.src_txid_list.empty())
    {
        throw std::runtime_error("交易来源异常");
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
        CRpcHelper::txfrom_info& rv = *it_s;
        CRpcHelper::TxDataInfo src = g_pRpcHelper->GetTransactionInfo_FromData( g_pRpcHelper->GetRawTransaction_FromTxId(rv.txid) );
        if (rv.vout >= 0 && rv.vout < src.dest_list.size())
        {
            const CRpcHelper::TxDataInfo::TxDestInfo& info = src.dest_list[rv.vout];
            from_addr_list.insert( info.addr.begin(), info.addr.end() );
            from_balance += info.value;
        }
    }
    if ( fabs(from_balance - to_amount) > ZERO_AMOUNT )
    {
        std::string sztip = str_format("警告：余额(%s)有可能不等于欲转账金额(%s)",
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
        if (s.size())
        {
            if (s[0] != '3' && s[0] != '2')
            {
                except_str += s + "\r\n";
            }
        }
    }
    if (except_str.size())
    {
        std::string sztip = str_format(
                                "警告，发现异常！\n正常应该是从担保地址转账出去的，现在发现疑似要从非担保地址地址%s 转账出去，确认还要继续？",
                                except_str.c_str()
                            );
        if (AfxMessageBox(sztip.c_str(), MB_ICONHAND | MB_YESNO | MB_DEFBUTTON2) == IDNO)
        {
            return false;
        }
    }
    std::string sztip = str_format("你确认要从%s，转币%s给对方么？", from_addr_str.c_str(), amount2str(to_amount).c_str());
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
        std::string txdata = GetWindowStlText(GetDlgItem(IDC_EDIT_TX_DATA));
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
		InsertLog("已确认发款完毕,建议上blockchain.info查一下是否有0确认的交易");
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
    sztip += str + "\r\n" + sztip;
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


double WebQuery_GetMulSignBalance(const std::string& addr)
{
	if (g_CoinAddr_FirstChar != '1')
	{
		throw std::runtime_error("not support coin type");
	}
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

CRpcHelper::txfrom_list WebQuery_GetTxFrom(const std::string& addr)
{
    // not support
    return CRpcHelper::txfrom_list();
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
                    SetDlgItemText(IDC_STATIC_PORT_STATE, "状态：已连接");

                    if (GetDlgItem(IDC_EDIT_PUBKEY_1)->GetWindowTextLength() == 0)
                    {
						std::string recvaddr = g_pRpcHelper->GetOrNewAccountAddress( g_pOption->PubAddrLabel );
						if (recvaddr.size())
						{
							g_CoinAddr_FirstChar = recvaddr[0];
						}
						else
						{
							g_CoinAddr_FirstChar = 0;
						}
                        std::string PubKey1 = g_pRpcHelper->GetPubKey(recvaddr);
                        assert( PubKey1.size() );
                        SetDlgItemText(IDC_EDIT_PUBKEY_1, PubKey1.c_str());

                    }
                }
                else if ( !HasCoinProcess(g_pOption) )
                {
                    CButton* pIsAutoStart = (CButton*)GetDlgItem(IDC_CHECK_IS_AUTO_START);
                    if ( pIsAutoStart->GetCheck() )
                    {
                        SetDlgItemText(IDC_STATIC_PORT_STATE, "状态：启动中");
                        StartProcess_FromOption(g_pOption);
                        SetDlgItemText(IDC_EDIT_PUBKEY_1, "");
                    }
                    else
                    {
                        SetDlgItemText(IDC_STATIC_PORT_STATE, "状态：等待连接");
                    }
                }
            }
            catch(std::exception& e)
            {
                std::string str = "错误,";
                str += e.what();
                PushError(str.c_str());
            }

            std::string ec = g_errorlist.PopString();
            if (ec.size())
            {
                InsertLog("错误:" + ec);
            }
            return;
        }
        else if (nIDEvent == TimerID_RefreshPubKey)
        {
            if ( Async_IsRpcCanConnected() )
            {
                std::string RealPubKey1 = g_pRpcHelper->GetPubKey(
                                              g_pRpcHelper->GetOrNewAccountAddress( g_pOption->PubAddrLabel ));
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
		addr = "https://blockchain.info/address/" + addr;
		ShellExecute(0, "open", addr.c_str(), NULL, NULL, SW_SHOW);
	}
}

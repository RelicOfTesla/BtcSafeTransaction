#pragma once

#include "json_rpc.hpp"
#include <map>
#include <set>
#include <list>
#include <vector>

enum RPCErrorCode
{
    // Standard JSON-RPC 2.0 errors
    RPC_INVALID_REQUEST  = -32600,
    RPC_METHOD_NOT_FOUND = -32601,
    RPC_INVALID_PARAMS   = -32602,
    RPC_INTERNAL_ERROR   = -32603,
    RPC_PARSE_ERROR      = -32700,

    // General application defined errors
    RPC_MISC_ERROR                  = -1,  // std::exception thrown in command handling
    RPC_FORBIDDEN_BY_SAFE_MODE      = -2,  // Server is in safe mode, and command is not allowed in safe mode
    RPC_TYPE_ERROR                  = -3,  // Unexpected type was passed as parameter
    RPC_INVALID_ADDRESS_OR_KEY      = -5,  // Invalid address or key
    RPC_OUT_OF_MEMORY               = -7,  // Ran out of memory during operation
    RPC_INVALID_PARAMETER           = -8,  // Invalid, missing or duplicate parameter
    RPC_DATABASE_ERROR              = -20, // Database error
    RPC_DESERIALIZATION_ERROR       = -22, // Error parsing or validating structure in raw format

    // P2P client errors
    RPC_CLIENT_NOT_CONNECTED        = -9,  // Bitcoin is not connected
    RPC_CLIENT_IN_INITIAL_DOWNLOAD  = -10, // Still downloading initial blocks

    // Wallet errors
    RPC_WALLET_ERROR                = -4,  // Unspecified problem with wallet (key not found etc.)
    RPC_WALLET_INSUFFICIENT_FUNDS   = -6,  // Not enough funds in wallet or account
    RPC_WALLET_INVALID_ACCOUNT_NAME = -11, // Invalid account name
    RPC_WALLET_KEYPOOL_RAN_OUT      = -12, // Keypool ran out, call keypoolrefill first
    RPC_WALLET_UNLOCK_NEEDED        = -13, // Enter the wallet passphrase with walletpassphrase first
    RPC_WALLET_PASSPHRASE_INCORRECT = -14, // The wallet passphrase entered was incorrect
    RPC_WALLET_WRONG_ENC_STATE      = -15, // Command given in wrong wallet encryption state (encrypting an encrypted wallet etc.)
    RPC_WALLET_ENCRYPTION_FAILED    = -16, // Failed to encrypt the wallet
    RPC_WALLET_ALREADY_UNLOCKED     = -17, // Wallet is already unlocked
};
class CRpcHelper
{
protected:
    shared_ptr<CCoinJsonRpc> m_rpc;
public:
    typedef std::string address_str;
    typedef std::string label_str;
    typedef std::string pubkey_str;
    typedef std::string txid_str;
    typedef std::string txdata_str;

    typedef std::set<std::string> str_unique_list;
    typedef std::map<address_str, label_str> mini_booklist;

    struct addr_ext_info
    {
        double balance;
        label_str label;

        addr_ext_info() : balance(0)
        {}
    };
    typedef std::map<address_str, addr_ext_info> full_booklist;

    struct txfrom_info
    {
        txid_str txid;
        size_t vout;

        txfrom_info() : vout(0)
        {}
    };
    typedef std::list<txfrom_info> txfrom_list;

    struct payout_record
    {
        double fAmount;
        address_str addr;

        payout_record() : fAmount(0)
        {}
    };

    typedef std::list<payout_record> payout_list;

    struct TxDataInfo
    {

		struct TxDestInfo
		{
			double value;
			str_unique_list addr;
		};

		typedef std::vector<TxDestInfo> TxDestList;

		txfrom_list src_txid_list;
		TxDestList dest_list;
    };
public:
    CRpcHelper(shared_ptr<CCoinJsonRpc> pRPC)
    {
        m_rpc = pRPC;
        m_cache_tick_getbalance = 0;
    }

    bool IsRpcCanConnected();

    pubkey_str GetPubKey(const pubkey_str& recvaddr);
    address_str NewMulSigAddr(const pubkey_str& PubKey1, const pubkey_str& PubKey2, const pubkey_str& PubKey3);

    void SetMulSigAddrLabel(const pubkey_str& PubKey1, const pubkey_str& PubKey2, const pubkey_str& PubKey3,
                            const label_str& label);

    full_booklist GetFullBookList();
    mini_booklist GetMiniBookList();

    double GetBalance_FromRecvAddr(const address_str& addr);
    label_str GetLabel(const address_str& addr);
    address_str GetOrNewAccountAddress(const label_str& label);

    void EnterPassword(const std::string& pass);

    txid_str SendAmount(const address_str& addr, double amount);

    txfrom_list GetUnspentTransactionList_FromRecvAddr(const address_str& addr);

    txdata_str CreateRawTransaction(const txfrom_list& txlist, const payout_list& paylist);

    txdata_str SignRawTransaction(const txdata_str& txdata);

    void SendRawTransaction(const txdata_str& txdata);

    TxDataInfo GetTransactionInfo_FromData(const txdata_str& txdata);
	txdata_str GetRawTransaction_FromTxId(const txid_str& txid);

	double GetRecvHistoryVolume_FromTxFrom(const txfrom_info& from);
public:
	TxDataInfo GetTransactionInfo_FromTxId(const txid_str& txid)
	{
		return GetTransactionInfo_FromData( GetRawTransaction_FromTxId(txid) );
	}
protected:

    address_str NewAddress(const label_str& label);
    str_unique_list GetLabelList();
    str_unique_list GetAddressList();
    str_unique_list GetAddrList_FromLabel(const label_str& label);

    void DupLabel(const address_str& addr, const label_str& label);

	Json::Value DecodeRawTransactionJson(const txdata_str& txdata);
private:
    UINT m_cache_tick_getbalance;
    full_booklist m_cache_list_getbalance;
};



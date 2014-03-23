#include "stdafx.h"
#include "rpc_helper.hpp"

#include <stack>

#define MAX_CONF 999999

bool CRpcHelper::IsRpcCanConnected()
{
	try
	{
		m_rpc->Send( "getgenerate" );
		return true;
	}
	catch( std::exception&)
	{}
	return false;
}

CRpcHelper::pubkey_str CRpcHelper::GetPubKey( const address_str& recvaddr )
{
	Json::Value param;
	param.append( recvaddr );
	Json::Value ret = json_from_string( m_rpc->Send( "validateaddress", param ) );
	return pubkey_str( ret["pubkey"].asString() );
}

CRpcHelper::address_str CRpcHelper::NewMulSigAddr(
	const pubkey_str& PubKey1,
	const pubkey_str& PubKey2,
	const pubkey_str& PubKey3 )
{
	Json::Value keys;
	keys.append( PubKey1 );
	keys.append( PubKey2 );
	if( PubKey3.size() )
	{
		keys.append( PubKey3 );
	}
	Json::Value args;
	args.append( 2 );
	args.append( keys );

	mini_booklist bakbook = GetMiniBookList();

	address_str ret = address_str( m_rpc->Send( "addmultisigaddress", args ) );

	label_str old_label = bakbook[ret];
	if( old_label.size() )
	{
		args.append( old_label );
		std::string ret = m_rpc->Send( "addmultisigaddress", args );
	}

	return ret;
}


void CRpcHelper::SetMulSigAddrLabel(
	const pubkey_str& PubKey1,
	const pubkey_str& PubKey2,
	const pubkey_str& PubKey3,
	const label_str& label )
{
	Json::Value keys;
	keys.append( PubKey1 );
	keys.append( PubKey2 );
	if( PubKey3.size() )
	{
		keys.append( PubKey3 );
	}
	Json::Value args;
	args.append( 2 );
	args.append( keys );
	args.append( label );

	std::string ret = m_rpc->Send( "addmultisigaddress", args );

}

CRpcHelper::mini_booklist CRpcHelper::GetMiniBookList()
{
	mini_booklist result;
#if 1
	Json::Value args;
	args.append( MAX_CONF );
	args.append( true );

	Json::Value jv = json_from_string( m_rpc->Send( "listreceivedbyaddress", args ) );
	for( Json::ValueIterator it = jv.begin(); it != jv.end(); ++it )
	{
		const Json::Value& rv = *it;
		bool ok = result.insert( std::make_pair( rv["address"].asString(), rv["account"].asString() ) ).second;
		assert( ok );

	}
#else
	str_unique_list labels = GetLabelList();
	for( str_unique_list::const_iterator it_label = labels.begin(); it_label != labels.end(); ++it_label )
	{
		str_unique_list label_addr_list = GetAddrList_FromLabel( *it_label );
		for( str_unique_list::const_iterator it_addr = label_addr_list.begin(); it_addr != label_addr_list.end(); ++it_addr )
		{
			bool ok = result.insert( std::make_pair( *it_addr, *it_label ) ).second;
			assert( ok );
		}
	}
#endif
	return result;
}


CRpcHelper::full_booklist CRpcHelper::GetFullBookList()
{
	full_booklist result;
	Json::Value ret = json_from_string( m_rpc->Send( "listaddressgroupings" ) );

	std::stack<Json::Value> stk;
	stk.push( ret );
	while( stk.size() )
	{
		const Json::Value rv = stk.top();
		stk.pop();
		if( rv.isArray() || rv.isObject() )
		{
			if( rv.size() > 0 )
			{
				if( rv[UINT( 0 )].isString() && rv.size() >= 2 )
				{
					assert( rv[1].isDouble() );
					addr_ext_info nv;
					address_str addr = address_str( rv[UINT( 0 )].asString() );
					nv.balance = rv[1].asDouble();
					if( rv.size() >= 3 )
					{
						nv.label = label_str( rv[2].asString() );
					}
					bool ok = result.insert( std::make_pair( addr, nv ) ).second;
					assert( ok );
				}
				else
				{
					for( Json::Value::const_iterator it = rv.begin(); it != rv.end(); ++it )
					{
						stk.push( *it );
					}
				}
			}
		}
		else
		{
			assert( 0 );
		}
	}

	mini_booklist minis = GetMiniBookList();
	for( mini_booklist::const_iterator it = minis.begin(); it != minis.end(); ++it )
	{
		addr_ext_info ar;
		ar.label = it->second;
		ar.balance = 0;
		result.insert( std::make_pair( it->first, ar ) ).second;

	}

	return result;
}

// not support mulsig address
double CRpcHelper::GetBalance_FromRecvAddr( const address_str& addr )
{
	if( GetTickCount() - m_cache_tick_getbalance > 1000 )
	{
		m_cache_list_getbalance = GetFullBookList();
		m_cache_tick_getbalance  = GetTickCount();
	}
	return m_cache_list_getbalance[addr].balance;
}

CRpcHelper::label_str CRpcHelper::GetLabel( const address_str& addr )
{
	Json::Value args;
	args.append( addr );
	std::string ret = m_rpc->Send( "getaccount", args );
	return label_str( ret );
}

void CRpcHelper::DupLabel( const address_str& addr, const label_str& label )
{
	Json::Value args;
	args.append( addr );
	args.append( label );
	std::string ret = m_rpc->Send( "setaccount", args );
}


CRpcHelper::txdata_str CRpcHelper::GetRawTransaction_FromTxId( const txid_str& txid )
{
	Json::Value args;
	args.append( txid );
	std::string ret = m_rpc->Send( "getrawtransaction", args );
	return txdata_str( ret );
}

void CRpcHelper::SetTxFee( double fee )
{
	Json::Value args;
	args.append( fee );
	std::string ret = m_rpc->Send( "settxfee", args );
}

std::pair<CRpcHelper::address_str, double> CRpcHelper::gettxout( const txid_str& txid, int n )
{
	std::pair<CRpcHelper::address_str, double> result;
	Json::Value args;
	args.append( txid );
	args.append( n );
	Json::Value jsv = json_from_string( m_rpc->Send( "gettxout", args ) );
	if( jsv.size() )
	{
		result.second = jsv["value"].asDouble();
		jsv = jsv["scriptPubKey"]["addresses"];
		if( jsv.size() )
		{
			result.first = address_str( jsv[UINT(0)].asString() );
			return result;
		}
	}
	throw std::runtime_error( "not coin in local database or was spend.");
}


double CRpcHelper::GetRecvHistoryVolume_FromTxFrom( const unk_txfrom_info& from )
{
	CRpcHelper::TxDataInfo query_tx = GetTransactionInfo_FromTxId( from.txid );
	if( from.vout >= 0 && from.vout < query_tx.dest_list.size() )
	{
		return query_tx.dest_list[from.vout].value;
	}
	else
	{
		assert( 0 );
		throw std::runtime_error( "error from.vout index" );
	}
}

Json::Value CRpcHelper::DecodeRawTransactionJson( const txdata_str& txdata )
{
	Json::Value args;
	args.append( txdata );
	std::string ret = m_rpc->Send( "decoderawtransaction", args );
	return json_from_string( ret );
}


CRpcHelper::address_str CRpcHelper::GetOrNewAccountAddress( const label_str& label )
{
#if 1
	Json::Value args;
	args.append( label );
	std::string ret = m_rpc->Send( "getaccountaddress", args );
	return address_str( ret );
#else
	if( GetAddrList_FromLabel( label ).empty() )
	{
		ret = NewAddress( label );
	}
	return address_str( ret );
#endif

}

void CRpcHelper::EnterPassword( const std::string& pass )
{
	Json::Value args;
	args.append( pass );
	args.append( 3 );
	std::string ret = m_rpc->Send( "walletpassphrase", args );
}

CRpcHelper::txid_str CRpcHelper::SendAmount( const address_str& addr, double fAmount )
{
	Json::Value args;
	args.append( addr );
	args.append( fAmount );
	std::string txid = m_rpc->Send( "sendtoaddress", args );
	return txid_str( txid );
}


CRpcHelper::unspent_info CRpcHelper::GetUnspentData_FromRecvAddr( const address_str& addr )
{
	// assert( !IsMulSigAddr(addr) ); // not support mulsig address

	Json::Value keys;
	keys.append( addr );

	Json::Value args;
	args.append( 1 );
	args.append( MAX_CONF );
	args.append( keys );
	const Json::Value jv = json_from_string( m_rpc->Send( "listunspent", args ) );

	unspent_info result;
	for( Json::Value::const_iterator it = jv.begin(); it != jv.end(); ++it )
	{
		const Json::Value& rv = *it;
		if( rv["address"] == addr )
		{
			unspent_info::unspent_txlist::value_type nv;
			nv.txid = txid_str( rv["txid"].asString() );
			nv.vout = rv["vout"].asInt();

			result.txlist.push_back( nv );
			result.total += rv["amount"].asDouble();
		}
	}
	return result;
}

CRpcHelper::txdata_str CRpcHelper::CreateRawTransaction( const unspent_info::unspent_txlist& txfromlist, const payout_list& paylist )
{

	Json::Value jv_txlist;
	for( auto it = txfromlist.begin(); it != txfromlist.end(); ++it )
	{
		auto& rv = *it;
		Json::Value nv;
		nv["txid"] = rv.txid;
		nv["vout"] = rv.vout;
		jv_txlist.append( nv );
	}

	Json::Value jv_payobj;
	for( payout_list::const_iterator it = paylist.begin(); it != paylist.end(); ++it )
	{
		const payout_record& rv = *it;
		jv_payobj[rv.addr] = rv.fAmount;
	}

	Json::Value args;
	args.append( jv_txlist );
	args.append( jv_payobj );

	std::string txdata = m_rpc->Send( "createrawtransaction", args );
	return txdata_str( txdata );
}

CRpcHelper::txdata_str CRpcHelper::SignRawTransaction( const txdata_str& txdata )
{
	Json::Value args;
	args.append( txdata );
	Json::Value jv = json_from_string( m_rpc->Send( "signrawtransaction", args ) );
	// assert( jv["complete"].asBool() ); // don't check it.
	return txdata_str( jv["hex"].asString() );
}

void CRpcHelper::SendRawTransaction( const txdata_str& txdata )
{
	Json::Value args;
	args.append( txdata );
	std::string ret = m_rpc->Send( "sendrawtransaction", args );
}


CRpcHelper::TxDataInfo CRpcHelper::GetTransactionInfo_FromData( const txdata_str& txdata )
{
	TxDataInfo result;
	Json::Value jv = DecodeRawTransactionJson( txdata );

	{
		const Json::Value& vin = jv["vin"];
		for( Json::Value::const_iterator it_i = vin.begin(); it_i != vin.end(); ++it_i )
		{
			const Json::Value& rv = *it_i;
			unk_txfrom_info from;
			from.txid = txid_str( rv["txid"].asString() );
			from.vout = rv["n"].asUInt();
			result.src_txid_list.push_back( from );
		}
	}

	{
		const Json::Value& vout = jv["vout"];
		for( Json::Value::const_iterator it_o = vout.begin(); it_o != vout.end(); ++it_o )
		{
			const Json::Value& rv = *it_o;
			TxDataInfo::TxDestInfo dst;
			dst.value = rv["value"].asDouble();
			const Json::Value& addrlist = rv["scriptPubKey"]["addresses"];
			for( Json::Value::const_iterator it_a = addrlist.begin(); it_a != addrlist.end(); ++it_a )
			{
				dst.addr.insert( ( *it_a ).asString() );
			}
			result.dest_list.push_back( dst );
		}
	}

	return result;
}

CRpcHelper::address_str CRpcHelper::NewAddress( const label_str& label )
{
	Json::Value args;
	args.append( label );
	std::string ret = m_rpc->Send( "getnewaddress", args );
	return address_str( ret );
}


CRpcHelper::str_unique_list CRpcHelper::GetLabelList()
{
	str_unique_list result;

	Json::Value args;
	args.append( MAX_CONF );
	const Json::Value labels = json_from_string( m_rpc->Send( "listaccounts", args ) );

	for( Json::Value::const_iterator it = labels.begin(); it != labels.end(); ++it )
	{
		bool ok = result.insert( it.key().asString() ).second;
		assert( ok );
	}
	return result;
}

CRpcHelper::str_unique_list CRpcHelper::GetAddressList()
{
	str_unique_list result;

#if 1
	Json::Value args;
	args.append( MAX_CONF );
	args.append( true );

	Json::Value jv = json_from_string( m_rpc->Send( "listreceivedbyaddress", args ) );
	for( Json::ValueIterator it = jv.begin(); it != jv.end(); ++it )
	{
		const Json::Value& rv = *it;
		bool ok = result.insert( rv["address"].asString() ).second;
		assert( ok );
	}
#else
	str_unique_list labels = GetLabelList();
	for( str_unique_list::const_iterator l_it = labels.begin(); l_it != labels.end(); ++l_it )
	{
		str_unique_list label_addr_list = GetAddrList_FromLabel( *l_it );
		for( str_unique_list::const_iterator a_it = label_addr_list.begin(); a_it != label_addr_list.end(); ++a_it )
		{
			bool ok = result.insert( *a_it ).second;
			assert( ok );
		}
	}
#endif
	return result;
}

CRpcHelper::str_unique_list CRpcHelper::GetAddrList_FromLabel( const label_str& label )
{
	str_unique_list result;

	Json::Value args;
	args.append( label );
	const Json::Value jv = json_from_string( m_rpc->Send( "getaddressesbyaccount", args ) );

	for( Json::Value::const_iterator it = jv.begin(); it != jv.end(); ++it )
	{
		bool ok = result.insert( ( *it ).asString() ).second;
		assert( ok );
	}
	return result;
}

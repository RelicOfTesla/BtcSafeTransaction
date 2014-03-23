#pragma once

#include <boost/noncopyable.hpp>
#include <sdk/json.h>
#include <comdef.h>

struct rpc_exception : std::runtime_error
{
	rpc_exception( const std::string& ec_str, INT error_code ) : std::runtime_error( ec_str )
	{
		m_error_code = error_code;
	}

	INT get_error_code()const
	{
		return m_error_code;
	}
	INT m_error_code;
};

class CCoinJsonRpc : boost::noncopyable
{
public:
	typedef USHORT RPC_PORT;

public:
	CCoinJsonRpc( const std::string& ip, RPC_PORT port, const std::string& username, const std::string& password );

public:
	std::string Send( const std::string& method, const Json::Value& params = Json::Value() ); // throw rpc_exception

	std::string SafeSendData( const std::string& str ); // throw std::exception
protected:
	bstr_t RawHttpRequest( const std::string& data );
protected:
	std::string m_url;
	std::string m_user, m_pass;
};
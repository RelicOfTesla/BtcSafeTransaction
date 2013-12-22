#include "stdafx.h"
#include "json_rpc.hpp"
#include <sdk/str_format.h>
#include <sdk/error.hpp>
#include <sdk/str_conv.hpp>
#import <winhttp.dll>

std::string AsciiCompressFromWCHAR(const wchar_t* src)
{
    std::string result;
    result.reserve(256);

    for (;;)
    {
        wchar_t wch = *src;
        ++src;
        if (wch == L'\0')
        {
            break;
        }
        else if (wch <= 0xff)
        {
            result += char(wch);
        }
        else
        {
            result += char(wch);
            result += char(wch >> 8);
        }
    }
    return result;
}
std::string BtcJsonWStr_BtcJsonAStr(const wchar_t* src)
{
    return AsciiCompressFromWCHAR(src);
}


std::string BtcJsonStr_To_JsonCppStr(const std::string& src)
{
    // 1. BTC Json: 0x80 char was not full convert to (\u) unicode data.    
	// 2. BTC Json \u string is UTF8-char array.  But JsonCpp \u data is unicode char.

    std::string result;
    result.reserve(src.size() + 100);
    std::string low_utf8_array;
    low_utf8_array.reserve(100);

    bool begin_str = false;
    for (size_t i = 0; i < src.size(); ++i)
    {
        BYTE uch = src[i];

        if (uch == '\'' || uch == '\"')
        {
            if (i >= 0 && src[i - 1] != '\\')
            {
                begin_str = !begin_str;
            }
        }
        if (begin_str)
        {
            if (uch >= 0x80)
            {
				// bitcoin-json was not >=0x80 char
                low_utf8_array += (char)uch;
                continue;
            }

            if (src.size() >= i + 5 + 1) // "\u0000"
            {
                if (uch == '\\' && src[i + 1] == 'u' && src[i + 2] == '0' && src[i + 3] == '0')
                {
					char szHEX[3];
					szHEX[0] = src[i+4];
					szHEX[1] = src[i+5];
					szHEX[2] = 0;
					uch = (BYTE)strtol(szHEX, nullptr, 16);

					low_utf8_array += (char)uch;

					i += 6 - 1;
					continue;
                }
            }
        }

		if (low_utf8_array.size())
		{
			result += UTF8_TO_ANSI(low_utf8_array.c_str());
			low_utf8_array.clear();
		}

        result += (char)uch;
    }
    return result;
}

std::string HttpGet(const std::string& url)
{
	try
	{
		WinHttp::IWinHttpRequestPtr p;
		p.CreateInstance("WinHttp.WinHttpRequest.5.1");
		p->Open("GET", url.c_str(), VARIANT_FALSE);
		p->Send(bstr_t());
		UINT state = p->Status;

		return std::string(p->ResponseText);
	}
	catch(_com_error& e)
	{
		throw std::runtime_error( ComError2Str<std::string>(e) );
	}
}
//////////////////////////////////////////////////////////////////////////
CCoinJsonRpc::CCoinJsonRpc(const std::string& ip, RPC_PORT port, const std::string& username, const std::string& password)
{
    m_url = str_format("http://%s:%d/", ip.c_str(), port);
    m_user = username;
    m_pass = password;
}

bstr_t CCoinJsonRpc::RawHttpRequest(const std::string& data)
{
    try
    {
        WinHttp::IWinHttpRequestPtr p;
        p.CreateInstance("WinHttp.WinHttpRequest.5.1");
        p->Open("POST", m_url.c_str(), VARIANT_FALSE);
        p->SetCredentials(m_user.c_str(), m_pass.c_str(), 0);
        p->Send(bstr_t(data.c_str()));
        UINT state = p->Status;

        return p->ResponseText;
    }
    catch(_com_error& e)
    {
        throw std::runtime_error( ComError2Str<std::string>(e) );
    }
}

std::string CCoinJsonRpc::SafeSendData(const std::string& data)
{
    // Bitcoin-json-rpc was not convert 128-255 ASCII to unicode.
	// don't use std::string x = bstr(httpdata) , was auto converted some error char.
    // so, use ASCII compress
    std::string str = BtcJsonWStr_BtcJsonAStr( RawHttpRequest(data) );

	
	str = BtcJsonStr_To_JsonCppStr( str );

	return str;

}

std::string CCoinJsonRpc::Send(const std::string& method, const Json::Value& params)
{
    Json::Value jsv;
    jsv["jsonrpc"] = "2.0";
    jsv["method"] = method;
    jsv["params"] = params;
    jsv["id"] = 0;

	std::string data = jsv.toStyledString();
	data = SafeSendData(data);
	Json::Value jval = json_from_string(data);
    if (jval.size() == 0)
    {
        std::string es = "HTTP REQUEST ERROR";
        es += "\r\n" + data;
        throw std::runtime_error(es);
    }
    Json::Value error_infos = jval["error"];
    if (error_infos.isObject() && error_infos.size() > 0 )
    {
        INT ec = error_infos["code"].asInt();
        std::string es = error_infos["message"].asString();
        throw rpc_exception( es, ec );
    }
    jval = jval["result"];
    if (jval.isObject() || jval.isArray())
    {
        return jval.toStyledString();
    }
    return jval.asString().c_str();
}

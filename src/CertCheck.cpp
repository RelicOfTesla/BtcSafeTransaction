#include "stdafx.h"
#include <Softpub.h>
#include <wintrust.h>
#include <sdk/binary.h>

#include <sdk/str_conv.hpp>

// Link with the Wintrust.lib file.


#pragma comment(lib, "crypt32.lib")

LONG( WINAPI* pfn_WinVerifyTrust )( HWND hwnd, GUID* pgActionID,
									LPVOID pWVTData );

bool IsFileCertNotChange( LPCWSTR pwszSourceFile )
{
	WINTRUST_FILE_INFO FileData;
	memset( &FileData, 0, sizeof( FileData ) );
	FileData.cbStruct = sizeof( WINTRUST_FILE_INFO );
	FileData.pcwszFilePath = pwszSourceFile;
	FileData.hFile = NULL;
	FileData.pgKnownSubject = NULL;


	WINTRUST_DATA WinTrustData;
	memset( &WinTrustData, 0, sizeof( WinTrustData ) );
	WinTrustData.cbStruct = sizeof( WinTrustData );
	WinTrustData.dwUIChoice = WTD_UI_NONE;
	WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
	WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
	WinTrustData.dwProvFlags = WTD_SAFER_FLAG;
	WinTrustData.pFile = &FileData;

	GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	*( FARPROC*)&pfn_WinVerifyTrust = GetProcAddress( LoadLibrary( "wintrust.dll" ), "WinVerifyTrust" );
	assert( pfn_WinVerifyTrust );
	LONG lStatus = pfn_WinVerifyTrust(
					   NULL,
					   &WVTPolicyGUID,
					   &WinTrustData );
	switch( lStatus )
	{
	case TRUST_E_NOSIGNATURE:
		return false;
	case ERROR_SUCCESS:
	default:
		return true;
	}
}

bool GetCertCheck( const std::string& filepath )
{
	std::wstring wstr = _A2W( filepath.c_str() );
	return IsFileCertNotChange( wstr.c_str() );
}

#include "stdafx.h"

#include "rpc_process.h"

#include <sdk/util/AppPath.hpp>
#include <sdk/error.hpp>
#include <sdk/str_format.h>

#include <Tlhelp32.h>
#include <Shlwapi.h>
#include "util.h"

//#include "option.h"

void StartProcess_FromOption( shared_ptr<CAppOption> opt )
{
	std::string BinPath, args;

	BinPath = GetFullFilePath( opt->Bin_DIR );
	if( opt->UseUIApp )
	{
		assert( opt->FileName_UIApp.size() );
		BinPath += "\\" + opt->FileName_UIApp;
	}
	else
	{
		assert( opt->FileName_ConsApp.size() );
		BinPath += "\\daemon\\" + opt->FileName_ConsApp;
	}

	args += str_format( " -server -rpcuser=%s -rpcpassword=%s -rpcport=%d",
						opt->LoginUser.c_str(), opt->LoginPass.c_str(), opt->Port );
	args += str_format( " \"-datadir=%s\" ", GetFullFilePath( opt->DB_DIR ).c_str() );

	args += " " + opt->StartExtern;

	STARTUPINFO si = {0};
	si.dwXSize = sizeof( si );
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;
	PROCESS_INFORMATION pi = {0};
	BOOL ok = CreateProcess( BinPath.c_str(), ( char*)args.c_str(), nullptr, nullptr, FALSE, 0, 0, GetDirectory( BinPath ).c_str(), &si, &pi );
	if( !ok )
	{
		throw std::logic_error( WinError2Str<std::string>() );
	}
	CloseHandle( pi.hThread );
	CloseHandle( pi.hProcess );
}


bool HasCoinProcess( shared_ptr<CAppOption> opt )
{
	std::string bindir = GetFullFilePath( opt->Bin_DIR );

	bool has = false;
	HANDLE hSnapProcess = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hSnapProcess )
	{
		PROCESSENTRY32 pe = {0};
		pe.dwSize = sizeof( pe );
		if( Process32First( hSnapProcess, &pe ) )
		{
			do
			{
				if( stricmp( pe.szExeFile, opt->FileName_UIApp.c_str() ) == 0 ||
						stricmp( pe.szExeFile, opt->FileName_ConsApp.c_str() ) == 0
				  )
				{
					HANDLE hSnapModule = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, pe.th32ProcessID );
					if( hSnapModule )
					{
						MODULEENTRY32 me = {0};
						me.dwSize = sizeof( me );
						if( Module32First( hSnapModule, &me ) )
						{
							if( StrStrI( me.szExePath, bindir.c_str() ) )
							{
								has = true;
							}
						}
						CloseHandle( hSnapModule );
					}
				}
			}
			while( Process32Next( hSnapProcess, &pe ) && !has );
		}

		CloseHandle( hSnapProcess );
	}
	return has;
}
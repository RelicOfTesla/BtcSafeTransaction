#pragma once

#include <sdk/shared_ptr.h>
#include <string>

struct CRpcOption
{

	std::string Bin_DIR, DB_DIR;

	std::string IP;
	UINT Port;

	std::string LoginUser, LoginPass;

	std::string StartExtern;


	BOOL UseUIApp;
	std::string FileName_UIApp, FileName_ConsApp;

};

struct CAppOption : CRpcOption
{
	BOOL DonateAuthor;

	std::string PubAddrLabel;

	double txfee;

	BOOL MakeRecvMode;
};

shared_ptr<CAppOption> LoadOption(const std::string& filepath);
void SaveOption(shared_ptr<CAppOption>, const std::string& filepath);

void InitMyAppOption();
void SaveMyAppOption(void);

extern shared_ptr<CAppOption> g_pOption;
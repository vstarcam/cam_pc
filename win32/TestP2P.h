// TestP2P.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CTestP2PApp:
// 有关此类的实现，请参阅 TestP2P.cpp
//

class CTestP2PApp : public CWinApp
{
public:
	CTestP2PApp();

// 重写
	public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CTestP2PApp theApp;
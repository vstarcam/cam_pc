// TestP2P.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTestP2PApp:
// �йش����ʵ�֣������ TestP2P.cpp
//

class CTestP2PApp : public CWinApp
{
public:
	CTestP2PApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTestP2PApp theApp;
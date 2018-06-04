// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT


#pragma warning(disable:4996)

#include "exeDebuger.h"


#ifdef __cplusplus
extern "C" {
#endif
    /*Include ffmpeg header file*/
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
    //#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

//ffmpeglib
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
//#pragma comment(lib, "swscale.lib")



#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if((p) != NULL) { delete (p); (p) = NULL; } }
#endif

#include <iostream>
using namespace std;
#include <vector>
#include <list>
#include "Afxmt.h"

#define K	1024
#define M	K*K
#define VIDEO_BUF 3*M
#define AUDIO_BUF 1*M

#include <afxsock.h>            // MFC �׽�����չ

#pragma warning(disable:4996)

#include "vfw.h"
#include <afxwin.h>
//#pragma comment(lib,"vfw32.lib")

#include <gdiplus.h>
//#include <stdio.h>
//using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")

#pragma warning(disable:4244)




#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif



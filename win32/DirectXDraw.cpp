#include "stdafx.h"
#include "DIRECTXDraw.h"

#include <stdio.h>
#include <Windows.h>
#include <Wingdi.h>

//#include "utility.h"
//#include "Bmp2Jpg.h"

#ifndef D1_VIDEO_WIDTH_PAL
#define	D1_VIDEO_WIDTH_PAL				720
#endif

#ifndef D1_VIDEO_HEIGHT_PAL
#define	D1_VIDEO_HEIGHT_PAL				576
#endif

#ifndef D1_VIDEO_HEIGHT_NTSC
#define	D1_VIDEO_HEIGHT_NTSC			480
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)					{ if(p != NULL) { delete (p);     (p) = NULL; } }   //Delete object by New create 
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)					{if(p != NULL) { (p)->Release(); (p) = NULL; } }
#endif

#ifndef CheckPointer
#define CheckPointer(pointer,hr)		{if(pointer == NULL) return hr;};
#endif

// colourspace conversion matrix values 
static unsigned __int64 mmw_mult_Y		= 0x2568256825682568;
static unsigned __int64 mmw_mult_U_G	= 0xf36ef36ef36ef36e;
static unsigned __int64 mmw_mult_U_B	= 0x40cf40cf40cf40cf;
static unsigned __int64 mmw_mult_V_R	= 0x3343334333433343;
static unsigned __int64 mmw_mult_V_G	= 0xe5e2e5e2e5e2e5e2;

// various masks and other constants 
static unsigned __int64 mmb_0x10		= 0x1010101010101010;
static unsigned __int64 mmw_0x0080		= 0x0080008000800080;
static unsigned __int64 mmw_0x00ff		= 0x00ff00ff00ff00ff;

// 5 Jan 2001  - Andrea Graziani    
static unsigned __int64 mask_5			= 0xf8f8f8f8f8f8f8f8;
static unsigned __int64 mask_6			= 0xfcfcfcfcfcfcfcfc;
static unsigned __int64 mask_blue		= 0x1f1f1f1f1f1f1f1f;

typedef struct tagTABLE_YUV2RGB
{
	unsigned short YtoR[256];
	unsigned short YtoG[256];
	unsigned short YtoB[256];		
	unsigned short UtoG[256];
	unsigned short UtoB[256];
	unsigned short VtoR[256];
	unsigned short VtoG[256];		
}TABLE_YUV2RGB;

static TABLE_YUV2RGB g_table_yuv2rgb;

//extern DISPLAYCALLBACK g_pDisplayCallBack;
//extern void* g_DisplayCallBackParam;

CDirectXDraw::CDirectXDraw(int nPlayHandle)
{
	m_hWndVideo			= NULL;
	m_rcOffset			= CRect(0,0,0,0);
	
	m_nVideoWidth		= D1_VIDEO_WIDTH_PAL;
	m_nVideoHeight		= D1_VIDEO_HEIGHT_PAL;
	m_bUseYUVDraw		= TRUE;
	m_pDD				= NULL;
	m_pddsFrontBuffer	= NULL;
	m_pddsBackBuffer	= NULL;
	m_bBltStretchX		= TRUE;
	m_bAutoCheck		= TRUE;

	m_bDrawImage		= FALSE;
	m_bSafeExit			= FALSE;

	memset(m_szSnapShotPath, 0, sizeof(m_szSnapShotPath));
	m_bSnapShot = FALSE;

	m_nPlayHandle = nPlayHandle;

	memset(m_szOSDText, 0, sizeof(m_szOSDText));
	m_nTextColor = 1;
	m_nBackColor = 0;
	m_bShowOSD = FALSE;
	m_nFontSize = 20;
	m_nX = 10;
	m_nY = 10;

	m_bDisplayVideoWidthRect = FALSE;
	m_nVideoRectX = 0;
	m_nVideoRectY = 0;
	m_nVideoRectWidth = 0;
	m_nVideoRectHeight = 0;
	
}

CDirectXDraw::~CDirectXDraw()
{
	if(m_bSafeExit)
		return;
	
	m_bSafeExit = TRUE;

	if(m_bDrawImage)
		Sleep(1);

	ReleaseObject();
}

void CDirectXDraw::DisplayVideoWithRect(BOOL bDisplay, int nX, int nY, int nWidth, int nHeight)
{
	m_bDisplayVideoWidthRect = bDisplay;
	if (bDisplay)
	{
		m_nVideoRectX = nX;
		m_nVideoRectY = nY;
		m_nVideoRectWidth = nWidth;
		m_nVideoRectHeight = nHeight;
	}
}

// 释放DirectDraw资源
void CDirectXDraw::ReleaseObject()
{
    SAFE_RELEASE( m_pddsBackBuffer );
    SAFE_RELEASE( m_pddsFrontBuffer );
	
    if( m_pDD )
        m_pDD->SetCooperativeLevel( 0, DDSCL_NORMAL );
	
    SAFE_RELEASE( m_pDD );
	
}

// 设置视频偏移区域
void CDirectXDraw::SetOffSetRect(CRect rcOffset)
{
	m_rcOffset = rcOffset;
}

// 设置显示窗口
BOOL CDirectXDraw::SetDisplayClipper(HWND hWndVideo)
{
	CheckPointer(m_pddsFrontBuffer,FALSE);
	CheckPointer(m_pDD,FALSE);
	CheckPointer(hWndVideo,FALSE);
	
	LPDIRECTDRAWCLIPPER pcClipper;
	if(FAILED(m_pDD->CreateClipper( 0, &pcClipper, NULL )))
		return FALSE;

	if(FAILED(pcClipper->SetHWnd(0,hWndVideo)))
	{
		pcClipper->Release();
		return FALSE;
	}

	if(FAILED(m_pddsFrontBuffer->SetClipper( pcClipper )))
	{
		pcClipper->Release();
		return FALSE;
	}

	pcClipper->Release();
	UpdateBounds();

	return TRUE;
}

// 更新区域
void CDirectXDraw::UpdateBounds()
{
	GetClientRect( m_hWndVideo, &m_rcVideo );

	//m_rcVideo.DeflateRect(2,2);
	ClientToScreen( m_hWndVideo, (POINT*)&m_rcVideo.left);
	ClientToScreen( m_hWndVideo, (POINT*)&m_rcVideo.right);

	m_rcVideo.DeflateRect(m_rcOffset);
}

// 创建YUV显示资源
BOOL CDirectXDraw::CreateSourceYUV(HWND hWndVideo,UINT nVideoWidth,UINT nVideoHeight)
{
	ReleaseObject();
	
	DDSURFACEDESC2 ddsd;
	DDCAPS         ddcap;
	
	if( FAILED( DirectDrawCreateEx( NULL, (VOID**)&m_pDD,IID_IDirectDraw7, NULL ) ) )
	{
		return FALSE;
	}
	
	if( FAILED(m_pDD->SetCooperativeLevel(hWndVideo, DDSCL_NORMAL )) )
	{
		return FALSE;
	}
	
	ZeroMemory(&ddcap,sizeof(ddcap));
	ddcap.dwSize = sizeof(ddcap);
	if(FAILED(m_pDD->GetCaps(&ddcap,NULL)))
	{
		return FALSE;
	}
	
	// 自动检测是否支持拉伸模式和显卡内存
	if(m_bAutoCheck)
	{
		if( (ddcap.dwFXCaps & DDFXCAPS_BLTSTRETCHX) && (ddcap.ddsCaps .dwCaps & DDSCAPS_VIDEOMEMORY))
			m_bBltStretchX = TRUE;
	}
	
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize			= sizeof(ddsd);
	ddsd.dwFlags		= DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if( FAILED( m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer, NULL ) ) )
	{
		return FALSE;
	}

	DDPIXELFORMAT  ddpfOverlayFormats = {sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0};  	
	
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize				= sizeof(ddsd);
	ddsd.dwFlags			= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps		= m_bBltStretchX ? (DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY) : (DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY);
	ddsd.dwWidth			= nVideoWidth;
	ddsd.dwHeight			= nVideoHeight;
	ddsd.ddpfPixelFormat	= ddpfOverlayFormats;
	if( FAILED( m_pDD->CreateSurface( &ddsd, &m_pddsBackBuffer, NULL ) ) )
	{
		return FALSE;
	}
	
	LPDIRECTDRAWCLIPPER pcClipper;
	if(FAILED(m_pDD->CreateClipper( 0, &pcClipper, NULL )))
		return FALSE;
	
	if(FAILED(pcClipper->SetHWnd( 0,hWndVideo)))
		return FALSE;
	
	if(FAILED(m_pddsFrontBuffer->SetClipper( pcClipper )))
		return FALSE;
	
	pcClipper->Release();
	
	DDBLTFX ddbltfx;
	ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
	ddbltfx.dwSize      = sizeof(ddbltfx);
	ddbltfx.dwFillColor = RGB(0,0,0);
	m_pddsBackBuffer->Blt( NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );
    
	UpdateBounds();
	m_emVideoMode	= VIDEO_MODE_YUV2;
	
	return TRUE;
}

// 创建RGB显示资源
BOOL CDirectXDraw::CreateSourceRGB(HWND hWndVideo,UINT nVideoWidth,UINT nVideoHeight)
{
	ReleaseObject();
	
	DDSURFACEDESC2 ddsd;
	DDCAPS         ddcap;
	
	if( FAILED( DirectDrawCreateEx( NULL, (VOID**)&m_pDD,IID_IDirectDraw7, NULL ) ) )
	{
		return FALSE;
	}

	if( FAILED(m_pDD->SetCooperativeLevel( 0, DDSCL_NORMAL )) )
	{
//		DW("DDRAW ERR:SetCooperativeLevel failed");  
		return FALSE;
	}
	
	ZeroMemory(&ddcap,sizeof(ddcap));
	ddcap.dwSize = sizeof(ddcap);
	if(FAILED(m_pDD->GetCaps(&ddcap,NULL)))
	{
//		DW("DDRAW ERR:GetCaps failed");
		return FALSE;
	}

	if(m_bAutoCheck)
	{
		if( (ddcap.dwFXCaps & DDFXCAPS_BLTSTRETCHX) && (ddcap.ddsCaps .dwCaps & DDSCAPS_VIDEOMEMORY))
			m_bBltStretchX = TRUE;
	}
	
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize			= sizeof(ddsd);
	ddsd.dwFlags		= DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if( FAILED( m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer, NULL ) ) )
	{
//		DW("DDRAW ERR:CreateMainSurface failed");  
		return FALSE;
	}

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize			= sizeof(ddsd);
	ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = m_bBltStretchX ? (DDSCAPS_OFFSCREENPLAIN) : (DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY);
	ddsd.dwWidth        = nVideoWidth;
	ddsd.dwHeight       = nVideoHeight;
	if( FAILED( m_pDD->CreateSurface( &ddsd, &m_pddsBackBuffer, NULL ) ) )
	{
//		DW("DDRAW ERR: CreateBackSurface failed");  
		return FALSE;
	}

	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize     = sizeof(DDSURFACEDESC2);
	if(FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL)))
	{
//		DW("DDRAW ERR:Surface Lock failed");
		return FALSE;
	}

	m_emVideoMode = VIDEO_MODE_NONE;	
	if ( ( ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB ) )
	{
		if(ddsd.ddpfPixelFormat.dwRGBBitCount == 32)
		{
			m_emVideoMode = VIDEO_MODE_RGB32;
		}
		else if(ddsd.ddpfPixelFormat.dwRGBBitCount == 24)
		{
			m_emVideoMode = VIDEO_MODE_RGB24;
		}
		else if(ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
		{
			if(ddsd.ddpfPixelFormat.dwRBitMask == 0xf800)
			{
				m_emVideoMode = VIDEO_MODE_RGB565;
			}
			else if(ddsd.ddpfPixelFormat.dwRBitMask==0x7C00)
			{
				m_emVideoMode = VIDEO_MODE_RGB555;
			}
			else
			{
				m_emVideoMode = VIDEO_MODE_NONE;
			}
		}
		else
		{
			m_pddsBackBuffer->Unlock(NULL);
//			DW("DDRAW ERR:Unknown Video Mode");
			return FALSE;
		}
	}
	else
	{
		m_pddsBackBuffer->Unlock(NULL);
//		DW("DDRAW ERR:Video Mode Is Not RGB Mode");
		return FALSE;
	}
	
	m_pddsBackBuffer->Unlock(NULL);
	
	if(m_emVideoMode == VIDEO_MODE_NONE)
	{
//		DW("DDRAW ERR:Unknown Video Mode");
		return FALSE;
	}
	
	return SetDisplayClipper(hWndVideo);
}

// 初始化DirectDraw
BOOL CDirectXDraw::InitDirectDraw(HWND hWndVideo,UINT nVideoWidth,UINT nVideoHeight,BOOL bUseYUVDraw)
{
	m_hWndVideo		= hWndVideo;
	m_nVideoWidth	= nVideoWidth;
	m_nVideoHeight	= nVideoHeight;	

	if(bUseYUVDraw)
	{
		if(!CreateSourceYUV(hWndVideo,nVideoWidth,nVideoHeight))	
			return FALSE;		
	}
	else
	{
		if(!CreateSourceRGB(hWndVideo,nVideoWidth,nVideoHeight))	
			return FALSE;	
	}
	
	m_bUseYUVDraw   = bUseYUVDraw;
	
	return TRUE;
}

// 显示视频
BOOL CDirectXDraw::DrawImageFromYUVBuf(	unsigned char *in_Y,
										unsigned char *in_U,
										unsigned char *in_V,
										unsigned int iWidth,
										unsigned int iHeight)
{	
	CheckPointer(m_pddsFrontBuffer,FALSE);
	CheckPointer(m_pddsBackBuffer,FALSE);
	
	if(m_bSafeExit)
		return TRUE;

	m_bDrawImage = TRUE;
	
	DDSURFACEDESC2  ddsd;
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize     = sizeof(DDSURFACEDESC2);
	if(FAILED(m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL)))
	{
		m_pddsBackBuffer->Restore();
		m_bDrawImage = FALSE;
		return FALSE;
	}
	
	if(in_U && in_V) 
	{
		// Y U V分量均有数据
		switch(m_emVideoMode)
		{
		case VIDEO_MODE_RGB32:
			YUVToRGB32(in_Y,iWidth,in_U,in_V,iWidth >> 1,
				(BYTE*)ddsd.lpSurface,iWidth,iHeight,ddsd.lPitch);
			break;
			
		case VIDEO_MODE_RGB24:
			YUVToRGB24(in_Y,iWidth,in_U,in_V,iWidth >> 1,
				(BYTE*)ddsd.lpSurface,iWidth,iHeight,ddsd.lPitch);
			break;
			
		case VIDEO_MODE_RGB555:
			YUVToRGB555(in_Y,iWidth,in_U,in_V,iWidth >> 1,
				(BYTE*)ddsd.lpSurface,iWidth,iHeight,ddsd.lPitch);
			break;
			
		case VIDEO_MODE_RGB565:
			YUVToRGB565(in_Y,iWidth,in_U,in_V,iWidth >> 1,
				(BYTE*)ddsd.lpSurface,iWidth,iHeight,ddsd.lPitch);
			break;
			
		case VIDEO_MODE_YUV2:
			YUV2(in_Y,iWidth,in_U,in_V,iWidth >> 1,
				(BYTE*)ddsd.lpSurface,iWidth,iHeight,ddsd.lPitch);
			
			break;
		}		
	}
	else if(in_Y)
	{
		// 只有Y分量数据
		for(int i=0; i<(int)ddsd.dwHeight; i++)
		{
			memcpy((BYTE*)ddsd.lpSurface + i * ddsd.lPitch, (BYTE*)in_Y + iWidth * i * 2, iWidth *  2);
		}
	}

	m_pddsBackBuffer->Unlock(NULL);

	//------ 显示OSD ---------------------------------------
	DrawOSD();

// 	//--------------------抓图 -------------------------------
// 	if (m_bSnapShot)
// 	{
// 		CapturePicture(iWidth, iHeight);
// 		m_bSnapShot = FALSE;
// 	}

	//--- 显示回调 -----------------------------------------------------
	HDC hBackDC;
	m_pddsBackBuffer->GetDC(&hBackDC);
// 	if (g_pDisplayCallBack)
// 	{
// 		g_pDisplayCallBack(m_nPlayHandle, hBackDC, g_DisplayCallBackParam);
// 	}
	m_pddsBackBuffer->ReleaseDC(hBackDC);

//--------------------------------------------------------------------------------
		
	UpdateBounds();

	CRect rcSrc;
	if (m_bDisplayVideoWidthRect)
	{
		rcSrc.left = m_nVideoRectX;
		rcSrc.top = m_nVideoRectY;
		rcSrc.right = rcSrc.left + m_nVideoRectWidth;
		rcSrc.bottom = rcSrc.top + m_nVideoRectHeight;
	}
	else
	{
		rcSrc.left = 0;
		rcSrc.top = 0;
		rcSrc.right = rcSrc.left + iWidth;
		rcSrc.bottom = rcSrc.top + iHeight;
	}

	if(m_pddsFrontBuffer->Blt(&m_rcVideo,m_pddsBackBuffer,&rcSrc,DDBLT_WAIT,NULL) == DDERR_SURFACELOST)
	{
		if(m_pddsFrontBuffer)
			m_pddsFrontBuffer->Restore();

		if(m_pddsBackBuffer)
			m_pddsBackBuffer->Restore();
	}
	
	m_bDrawImage = FALSE;

	return TRUE;
}

void CDirectXDraw::DrawOSD()
{
	if (!m_bShowOSD)
	{
		return ;
	}	

	HDC hBackDC;
	m_pddsBackBuffer->GetDC(&hBackDC);	

	HFONT	hFont, hOldFont;
	hFont= CreateFont(-m_nFontSize, 0, 0, 0, 400, FALSE, FALSE, FALSE, 134, 0, 0, 0, 2, "宋体");//"宋体");
	hOldFont = (HFONT)SelectObject(hBackDC, hFont);

	
	switch (m_nTextColor)
	{
	case 1: SetTextColor(hBackDC, RGB(0, 0, 0)); break;
	case 2: SetTextColor(hBackDC, RGB(255, 255, 0)); break;
	case 3: SetTextColor(hBackDC, RGB(255, 0, 0)); break;
	case 4: SetTextColor(hBackDC, RGB(255, 255, 255)); break;
	case 5: SetTextColor(hBackDC, RGB(0, 0, 255)); break;
	}

	switch (m_nBackColor)
	{
	case 0: SetBkMode(hBackDC, TRANSPARENT);
	case 1: SetBkColor(hBackDC, RGB(0, 0, 0)); break;
	case 2: SetBkColor(hBackDC, RGB(255, 255, 0)); break;
	case 3: SetBkColor(hBackDC, RGB(255, 0, 0)); break;
	case 4: SetBkColor(hBackDC, RGB(255, 255, 255)); break;
	case 5: SetBkColor(hBackDC, RGB(0, 0, 255)); break;
	}

	TextOut(hBackDC, m_nX, m_nY, m_szOSDText, strlen(m_szOSDText));

	SelectObject(hBackDC, hOldFont);
	DeleteObject(hFont);

	m_pddsBackBuffer->ReleaseDC(hBackDC);
}

void CDirectXDraw::SnapShot(char *szPath)
{
	memset(m_szSnapShotPath, 0, sizeof(m_szSnapShotPath));
	strcpy(m_szSnapShotPath, szPath);

	m_bSnapShot = TRUE;
}
// 
// void CDirectXDraw::CapturePicture(int iWidth, int iHeight)
// {
// 	HDC hBackDC;
// 	m_pddsBackBuffer->GetDC(&hBackDC);
// 
// 	CDC *pDC;//屏幕DC
// 	pDC = CDC::FromHandle(GetDC(NULL));//获取当前整个屏幕DC
// 	int BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);//获得颜色模式
// 
// 	CDC memDC;//内存DC
// 	memDC.CreateCompatibleDC(pDC);
// 	CDC *pBackDC = CDC::FromHandle(hBackDC);
// 
// 
// 	CBitmap memBitmap, *oldmemBitmap;//建立和屏幕兼容的bitmap
// 	memBitmap.CreateCompatibleBitmap(pDC, iWidth, iHeight);
// 	oldmemBitmap = memDC.SelectObject(&memBitmap);//将memBitmap选入内存DC
// 	memDC.BitBlt(0, 0, iWidth, iHeight, pBackDC, 0, 0, SRCCOPY);//复制屏幕图像到内存DC
// 
// 	//以下代码保存memDC中的位图到文件
// 
// 	BITMAP bmp;
// 	memBitmap.GetBitmap(&bmp);//获得位图信息
// 
// 	//创建目录
// 	CreateSnapShotDirectory(m_szSnapShotPath);
// 
// //	FILE *fp = fopen(m_szSnapShotPath, "wb");
// //	ASSERT(fp != NULL);
// 
// 	int BmpLength = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmp.bmWidthBytes * bmp.bmHeight;
// 	char *pBuf = new char[BmpLength];
// 	ASSERT(pBuf);
// 	
// //	if (fp != NULL)
// 	if (pBuf != NULL)
// 	{
// 		BITMAPINFOHEADER bih = {0};//位图信息头
// 		bih.biBitCount = bmp.bmBitsPixel;//每个像素字节大小
// 		bih.biCompression = BI_RGB;
// 		bih.biHeight = bmp.bmHeight;//高度
// 		bih.biPlanes = 1;
// 		bih.biSize = sizeof(BITMAPINFOHEADER);
// 		bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//图像数据大小
// 		bih.biWidth = bmp.bmWidth;//宽度
// 
// 
// 		BITMAPFILEHEADER bfh = {0};//位图文件头
// 		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//到位图数据的偏移量
// 		bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//文件总的大小
// 		bfh.bfType = (WORD)0x4d42;
// 
// //		fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);//写入位图文件头 
// //		fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);//写入位图信息头
// 		char *pTmp = pBuf;
// 		memcpy(pTmp, (char*)&bfh, sizeof(BITMAPFILEHEADER));
// 		pTmp += sizeof(BITMAPFILEHEADER);
// 		memcpy(pTmp, (char*)&bih, sizeof(BITMAPINFOHEADER));
// 		pTmp += sizeof(BITMAPINFOHEADER);
// 
// 		byte * p = new byte[bmp.bmWidthBytes * bmp.bmHeight];//申请内存保存位图数据
// 
// 		GetDIBits(memDC.m_hDC, (HBITMAP) memBitmap.m_hObject, 0, iHeight, p,
// 			(LPBITMAPINFO) &bih, DIB_RGB_COLORS);//获取位图数据
// 
// //		fwrite(p, 1, sizeof(BITMAPINFOHEADER), fp);//写入位图数据
// 		memcpy(pTmp, p, bmp.bmWidthBytes * bmp.bmHeight);
// 
// //		delete [] p;
// 		SAFE_DELETE(p);
// //		fclose(fp);
// 
// 		CBmp2Jpg Bmp2Jpg;
// 		Bmp2Jpg.BMP2JPG(m_szSnapShotPath, pBuf, BmpLength);
// 
// 		SAFE_DELETE(pBuf);
// 
// 	}
// 	
// 	memDC.SelectObject(oldmemBitmap);
// 	m_pddsBackBuffer->ReleaseDC(hBackDC);
// }

BOOL CDirectXDraw::CreateSnapShotDirectory(char *szPath)
{
	WIN32_FIND_DATA   fdData;   
	CString strPath = "";

	strPath.Format("%s", szPath);

	strPath.Replace("/","\\");

	int iPos = strPath.Find("\\");
	while (-1 != iPos)
	{
		CString strTmp = "";
		strTmp = strPath.Left(iPos);

		HANDLE   hFind=::FindFirstFile(strTmp,&fdData);   
		//如果源文件（包括文件夹)不存在，创建文件   
		if(hFind   ==   INVALID_HANDLE_VALUE)   
		{     
			::CreateDirectory(strTmp,NULL);   
		}   
		::FindClose(hFind);

		iPos = strPath.Find("\\", iPos + 1);
	}

	return TRUE;
}

void CDirectXDraw::UYVY2YUYV(LPBYTE pBuf,int cbSize)
{
	_asm
	{							
		push	eax
		push	ebx
		push	ecx
		push	edx
		
		mov		ecx,dword ptr[cbSize];//loop times
		xor		ebx,ebx//ebx=i
comp:
		cmp		ebx,ecx
		jge		Finished_UYVY
		
		//loop begin
		mov		eax,dword ptr[pBuf]
		mov		eax,dword ptr[eax+ebx]
		
		//eax为UYVY布局
		mov		edx,eax
		
		//转edx部分
		shr		edx,16
		rol		dx,8
		shl		edx,16
		
		//转eax部分
		rol		ax,8
		and		eax,0xFFFF
		or		eax,edx
		
		//合并为YUYV
		mov		edx,dword ptr[pBuf]
		mov		dword ptr[edx+ebx],eax
		
		add		ebx,4
		jmp		comp
			
Finished_UYVY:
		pop		edx
		pop		ecx
		pop 	ebx
		pop		eax
	}
}

void CDirectXDraw::InitYUV2RGBTable()
{
	static BOOL bInited = FALSE;
	if(bInited)
		return;
	
	bInited = TRUE;
	
	// calculate yuv-rgb look up table, left shift 7 bits
	for (unsigned short i = 0; i < 256; ++i)
	{			
		g_table_yuv2rgb.YtoR[i] = g_table_yuv2rgb.YtoG[i] = g_table_yuv2rgb.YtoB[i] 
			
			= (unsigned short)(i << 7);
		
		g_table_yuv2rgb.VtoR[i] = (unsigned short)(i * 180);
		
		g_table_yuv2rgb.VtoG[i] = (unsigned short)(i * 91);
		
		g_table_yuv2rgb.UtoG[i] = (unsigned short)(i * 44);
		
		g_table_yuv2rgb.UtoB[i] = (unsigned short)(i * 226);
		
	}
}

BYTE *CDirectXDraw::GetBufferPtr()
{
	DDSURFACEDESC2  ddsd;
	HRESULT         ddrval;

	if ((m_pddsFrontBuffer==NULL) || (m_pddsBackBuffer==NULL))
	{
		TRACE("CDirectDisplay::GetBufferPtr(%x) frontbuf %x backbuf %x\n", this, m_pddsFrontBuffer, m_pddsBackBuffer);
		return NULL;
	}

	if (IsWindow(m_hWndVideo) == FALSE)
	{
		TRACE("CDirectDisplay::GetBufferPtr(%x) m_hWnd is not window\n", this);
		return NULL;
	}

	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddrval		= m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (ddrval != DD_OK)
	{
		TRACE("CDirectDisplay::GetBufferPtr(%x) backbuf Lock failure %x\n", this, ddrval);
		if (ddrval == DDERR_SURFACELOST)
		{	
			ddrval = m_pddsFrontBuffer->Restore();
			ddrval = m_pddsBackBuffer->Restore();
			if (DD_OK != ddrval)
			{
				TRACE("CDirectDisplay::GetBufferPtr(%x) restore failure\n", this);
				return NULL;
			}
			m_pDD->RestoreAllSurfaces();
		}

		return NULL;
	}

//	if (IsWindowVisible(m_hWndVideo) == FALSE)
	{
		m_pddsBackBuffer->Unlock(NULL);
		TRACE("CDirectDisplay::GetBufferPtr(%x) m_hWnd is not window visible\n", this);		
//		return NULL;
	}


	return (BYTE *)ddsd.lpSurface;
}

void CDirectXDraw::YUY2ToRGB24(const unsigned char* pSrc, DWORD dwSrc,unsigned char* pDst, DWORD dwDst)
{
	InitYUV2RGBTable();
	
	ASSERT((dwSrc&0x11) == 0);//%4 == 0
	
	DWORD m = 0;
	DWORD k = 0;
	
	int tmpR0 = 0;
	int tmpG0 = 0;
	int tmpB0 = 0;
	int tmpR1 = 0;
	int tmpG1 = 0;
	int tmpB1 = 0;
	
	for (; (k+3) < dwSrc;)
	{
		ASSERT(m+5<dwDst);
		tmpR0 = (g_table_yuv2rgb.YtoR[pSrc[k + 0]] + g_table_yuv2rgb.VtoR[pSrc[k + 3]] - 22906) >> 7;
		tmpG0 = (g_table_yuv2rgb.YtoG[pSrc[k + 0]] - g_table_yuv2rgb.VtoG[pSrc[k + 3]] - g_table_yuv2rgb.UtoG[pSrc[k + 1]] + 17264) >> 7;
		tmpB0 = (g_table_yuv2rgb.YtoB[pSrc[k + 0]] + g_table_yuv2rgb.UtoB[pSrc[k + 1]] - 28928) >> 7;
		
		tmpR1 = (g_table_yuv2rgb.YtoR[pSrc[k + 2]] + g_table_yuv2rgb.VtoR[pSrc[k + 3]] - 22906) >> 7;		
		tmpG1 = (g_table_yuv2rgb.YtoG[pSrc[k + 2]] - g_table_yuv2rgb.VtoG[pSrc[k + 3]] - g_table_yuv2rgb.UtoG[pSrc[k + 1]] + 17264) >> 7;		
		tmpB1 = (g_table_yuv2rgb.YtoB[pSrc[k + 2]] + g_table_yuv2rgb.UtoB[pSrc[k + 1]] - 28928) >> 7;		
		
		if (tmpR0 > 255) tmpR0 = 255;
		if (tmpG0 > 255) tmpG0 = 255;
		if (tmpB0 > 255) tmpB0 = 255;
		if (tmpR1 > 255) tmpR1 = 255;
		if (tmpG1 > 255) tmpG1 = 255;
		if (tmpB1 > 255) tmpB1 = 255;
		
		if (tmpR0 < 0) tmpR0 = 0;
		if (tmpG0 < 0) tmpG0 = 0;
		if (tmpB0 < 0) tmpB0 = 0;
		if (tmpR1 < 0) tmpR1 = 0;
		if (tmpG1 < 0) tmpG1 = 0;
		if (tmpB1 < 0) tmpB1 = 0;
		
		//*
		pDst[m + 0] = (unsigned char)tmpB0;
		pDst[m + 1] = (unsigned char)tmpG0;
		pDst[m + 2] = (unsigned char)tmpR0;
		pDst[m + 3] = (unsigned char)tmpB1;
		pDst[m + 4] = (unsigned char)tmpG1;
		pDst[m + 5] = (unsigned char)tmpR1;
		k += 4;
		m += 6;
	}
}

void CDirectXDraw::YUVTOBMP24(BYTE *puc_y, int stride_y, 
							 BYTE *puc_u, BYTE *puc_v, int stride_uv, 
							 BYTE *puc_out, int width_y, int height_y,
							 unsigned int _stride_out)
{
	int y, horiz_count;
	horiz_count = -(width_y >> 3);
	
	for (y=0; y<height_y; y+=2) 
	{
		_asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
horiz_loop1:
			
			movd mm2, [ecx]
			pxor mm7, mm7
			
			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			
			psubusb mm0, mmb_0x10    ; mm0 -= 16
			
			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0
			
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8
			
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8
			
			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8
			
			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v
			
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u
			
			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part
			
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0
			
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0
			
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1
			
			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma
			
			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0
			
			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1
			
			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1
			
			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1
			
			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0
			
			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpcklbw mm1, mm0       ; mm1 = __g3__g2__g1__g0
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpcklbw mm0, mm7       ; mm0 = r3b3r2b2r1b1r0b0
			
			movq mm2, mm0            ; mm2 = r3b3r2b2r1b1r0b0
			
			punpcklbw mm0, mm1       ; mm0 = __r1g1b1__r0g0b0
			punpckhbw mm2, mm1       ; mm2 = __r3g3b3__r2g2b2
			
			/* 24-bit shuffle and save... */
			movd   [eax], mm0        ; eax[0] = __r0g0b0
			psrlq mm0, 32            ; mm0 = __r1g1b1
			
			movd  3[eax], mm0        ; eax[3] = __r1g1b1
			
			movd  6[eax], mm2        ; eax[6] = __r2g2b2
			
			
			psrlq mm2, 32            ; mm2 = __r3g3b3
			
			movd  9[eax], mm2        ; eax[9] = __r3g3b3
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpckhbw mm1, mm0       ; mm1 = __g7__g6__g5__g4
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpckhbw mm0, mm7       ; mm0 = r7b7r6b6r5b5r4b4
			
			movq mm2, mm0            ; mm2 = r7b7r6b6r5b5r4b4
			
			punpcklbw mm0, mm1       ; mm0 = __r5g5b5__r4g4b4
			punpckhbw mm2, mm1       ; mm2 = __r7g7b7__r6g6b6
			
			/* 24-bit shuffle and save... */
			movd 12[eax], mm0        ; eax[12] = __r4g4b4
			psrlq mm0, 32            ; mm0 = __r5g5b5
			
			movd 15[eax], mm0        ; eax[15] = __r5g5b5
			add ebx, 8               ; puc_y   += 8;
			
			movd 18[eax], mm2        ; eax[18] = __r6g6b6
			psrlq mm2, 32            ; mm2 = __r7g7b7
				
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;
			
			movd 21[eax], mm2        ; eax[21] = __r7g7b7
			add eax, 24              ; puc_out += 24
			
			inc edi
			jne horiz_loop1			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms
		}

		puc_out += _stride_out; 
        puc_y   -= stride_y;
        _asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
horiz_loop2:
			
			movd mm2, [ecx]
			pxor mm7, mm7
			
			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			
			psubusb mm0, mmb_0x10    ; mm0 -= 16
			
			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0
			
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8
			
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8
			
			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8
			
			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v
			
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u
			
			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part
			
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0
			
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0
			
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1
			
			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma
			
			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0
			
			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1
			
			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1
			
			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1
			
			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0
			
			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpcklbw mm1, mm0       ; mm1 = __g3__g2__g1__g0
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpcklbw mm0, mm7       ; mm0 = r3b3r2b2r1b1r0b0
			
			movq mm2, mm0            ; mm2 = r3b3r2b2r1b1r0b0
			
			punpcklbw mm0, mm1       ; mm0 = __r1g1b1__r0g0b0
			punpckhbw mm2, mm1       ; mm2 = __r3g3b3__r2g2b2
			
			/* 24-bit shuffle and save... */
			movd   [eax], mm0        ; eax[0] = __r0g0b0
			psrlq mm0, 32            ; mm0 = __r1g1b1
			
			movd  3[eax], mm0        ; eax[3] = __r1g1b1
			
			movd  6[eax], mm2        ; eax[6] = __r2g2b2
			psrlq mm2, 32            ; mm2 = __r3g3b3
			
			movd  9[eax], mm2        ; eax[9] = __r3g3b3
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpckhbw mm1, mm0       ; mm1 = __g7__g6__g5__g4
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpckhbw mm0, mm7       ; mm0 = r7b7r6b6r5b5r4b4
			
			movq mm2, mm0            ; mm2 = r7b7r6b6r5b5r4b4
			
			punpcklbw mm0, mm1       ; mm0 = __r5g5b5__r4g4b4
			punpckhbw mm2, mm1       ; mm2 = __r7g7b7__r6g6b6
			
			/* 24-bit shuffle and save... */
			movd 12[eax], mm0        ; eax[12] = __r4g4b4
			psrlq mm0, 32            ; mm0 = __r5g5b5
			
			movd 15[eax], mm0        ; eax[15] = __r5g5b5
			add ebx, 8               ; puc_y   += 8;
			
			movd 18[eax], mm2        ; eax[18] = __r6g6b6
			psrlq mm2, 32            ; mm2 = __r7g7b7
			
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;
			
			movd 21[eax], mm2        ; eax[21] = __r7g7b7
			add eax, 24              ; puc_out += 24
			
			inc edi
			jne horiz_loop2			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms
		} 

		puc_out += _stride_out; 
        puc_y   -= stride_y;
		puc_u   -= stride_uv;
		puc_v   -= stride_uv;
	}
}

// YUV -> RGB conversion, 32-bit output
// if height_y is negative then the output image will be inverted
// note: _stride_out parameter is ignored in yuv to rgb conversion 
// it's assumed that stride_out = 4 * width_y for the 32 bit color bitmap
void CDirectXDraw::YUVToRGB32(BYTE *puc_y, int stride_y, 
								BYTE *puc_u, BYTE *puc_v, int stride_uv, 
								BYTE *puc_out, int width_y, int height_y,
								unsigned int _stride_out) 
{
	
	int y, horiz_count;
	horiz_count = -(width_y >> 3);
	
	for (y=0; y<height_y; y+=2) 
	{
		_asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
				
horiz_loop1:
			
			movd mm2, [ecx]
			pxor mm7, mm7
			
			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			
			psubusb mm0, mmb_0x10    ; mm0 -= 16   //成组数据相减
			
			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0
			
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8
			
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8
			
			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8
			
			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v
			
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u
			
			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part
			
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0
			
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0
			
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1
			
			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma
			
			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0
			
			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1
			
			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1
			
			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1
			
			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0
			
			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpcklbw mm1, mm0       ; mm1 = __g3__g2__g1__g0
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpcklbw mm0, mm7       ; mm0 = r3b3r2b2r1b1r0b0
			
			movq mm2, mm0            ; mm2 = r3b3r2b2r1b1r0b0
			
			punpcklbw mm0, mm1       ; mm0 = __r1g1b1__r0g0b0
			punpckhbw mm2, mm1       ; mm2 = __r3g3b3__r2g2b2
			
			/* 32-bit save... */
			movq  [eax], mm0         ; eax[0] = __r1g1b1__r0g0b0
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			
			movq 8[eax], mm2         ; eax[8] = __r3g3b3__r2g2b2
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			punpckhbw mm1, mm0       ; mm1 = __g7__g6__g5__g4
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpckhbw mm0, mm7       ; mm0 = r7b7r6b6r5b5r4b4
			
			movq mm2, mm0            ; mm2 = r7b7r6b6r5b5r4b4
			
			punpcklbw mm0, mm1       ; mm0 = __r5g5b5__r4g4b4
			punpckhbw mm2, mm1       ; mm2 = __r7g7b7__r6g6b6
			
			/* 32-bit save... */
			add ebx, 8               ; puc_y   += 8;
			add ecx, 4               ; puc_u   += 4;
			
			movq 16[eax], mm0        ; eax[16] = __r5g5b5__r4g4b4
			add edx, 4               ; puc_v   += 4;
			
			movq 24[eax], mm2        ; eax[24] = __r7g7b7__r6g6b6
				
			// 0 1 2 3 4 5 6 7 rgb save order
			
			add eax, 32              ; puc_out += 32
			
			inc edi
			jne horiz_loop1			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms
			
		} 
		puc_out += _stride_out;
		puc_y   += stride_y;
        _asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
				
horiz_loop2:
			
			movd mm2, [ecx]
			pxor mm7, mm7
			
			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			
			psubusb mm0, mmb_0x10    ; mm0 -= 16   //成组数据相减
			
			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0
			
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8
			
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8
			
			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8
			
			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v
			
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u
			
			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part
			
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0
			
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0
			
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1
			
			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma
			
			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0
			
			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1
			
			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1
			
			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1
			
			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0
			
			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpcklbw mm1, mm0       ; mm1 = __g3__g2__g1__g0
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpcklbw mm0, mm7       ; mm0 = r3b3r2b2r1b1r0b0
			
			movq mm2, mm0            ; mm2 = r3b3r2b2r1b1r0b0
			
			punpcklbw mm0, mm1       ; mm0 = __r1g1b1__r0g0b0
			punpckhbw mm2, mm1       ; mm2 = __r3g3b3__r2g2b2
			
			/* 32-bit save... */
			movq  [eax], mm0         ; eax[0] = __r1g1b1__r0g0b0
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			
			movq 8[eax], mm2         ; eax[8] = __r3g3b3__r2g2b2
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			punpckhbw mm1, mm0       ; mm1 = __g7__g6__g5__g4
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpckhbw mm0, mm7       ; mm0 = r7b7r6b6r5b5r4b4
			
			movq mm2, mm0            ; mm2 = r7b7r6b6r5b5r4b4
			
			punpcklbw mm0, mm1       ; mm0 = __r5g5b5__r4g4b4
			punpckhbw mm2, mm1       ; mm2 = __r7g7b7__r6g6b6
			
			/* 32-bit save... */
			add ebx, 8               ; puc_y   += 8;
			add ecx, 4               ; puc_u   += 4;
			
			movq 16[eax], mm0        ; eax[16] = __r5g5b5__r4g4b4
			add edx, 4               ; puc_v   += 4;
			
			movq 24[eax], mm2        ; eax[24] = __r7g7b7__r6g6b6
				
			// 0 1 2 3 4 5 6 7 rgb save order
			
			add eax, 32              ; puc_out += 32
			
			inc edi
			jne horiz_loop2			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms
					
		}
		
		puc_out += _stride_out;
		puc_y   += stride_y;
		puc_u   += stride_uv;
		puc_v   += stride_uv;
	}
}

// YUV -> RGB conversion, 24-bit output
void CDirectXDraw::YUVToRGB24(BYTE *puc_y, int stride_y, 
										BYTE *puc_u, BYTE *puc_v, int stride_uv, 
										BYTE *puc_out, int width_y, int height_y,
										unsigned int _stride_out) 
{
	
	int y, horiz_count;
	horiz_count = -(width_y >> 3);
	
	for (y=0; y<height_y; y+=2) 
	{
		_asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
horiz_loop1:
			
			movd mm2, [ecx]
			pxor mm7, mm7
			
			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			
			psubusb mm0, mmb_0x10    ; mm0 -= 16
			
			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0
			
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8
			
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8
			
			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8
			
			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v
			
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u
			
			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part
			
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0
			
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0
			
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1
			
			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma
			
			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0
			
			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1
			
			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1
			
			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1
			
			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0
			
			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpcklbw mm1, mm0       ; mm1 = __g3__g2__g1__g0
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpcklbw mm0, mm7       ; mm0 = r3b3r2b2r1b1r0b0
			
			movq mm2, mm0            ; mm2 = r3b3r2b2r1b1r0b0
			
			punpcklbw mm0, mm1       ; mm0 = __r1g1b1__r0g0b0
			punpckhbw mm2, mm1       ; mm2 = __r3g3b3__r2g2b2
			
			/* 24-bit shuffle and save... */
			movd   [eax], mm0        ; eax[0] = __r0g0b0
			psrlq mm0, 32            ; mm0 = __r1g1b1
			
			movd  3[eax], mm0        ; eax[3] = __r1g1b1
			
			movd  6[eax], mm2        ; eax[6] = __r2g2b2
			
			psrlq mm2, 32            ; mm2 = __r3g3b3
			
			movd  9[eax], mm2        ; eax[9] = __r3g3b3
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpckhbw mm1, mm0       ; mm1 = __g7__g6__g5__g4
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpckhbw mm0, mm7       ; mm0 = r7b7r6b6r5b5r4b4
			
			movq mm2, mm0            ; mm2 = r7b7r6b6r5b5r4b4
			
			punpcklbw mm0, mm1       ; mm0 = __r5g5b5__r4g4b4
			punpckhbw mm2, mm1       ; mm2 = __r7g7b7__r6g6b6
			
			/* 24-bit shuffle and save... */
			movd 12[eax], mm0        ; eax[12] = __r4g4b4
			psrlq mm0, 32            ; mm0 = __r5g5b5
			
			movd 15[eax], mm0        ; eax[15] = __r5g5b5
			add ebx, 8               ; puc_y   += 8;
			
			movd 18[eax], mm2        ; eax[18] = __r6g6b6
			psrlq mm2, 32            ; mm2 = __r7g7b7
				
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;
			
			movd 21[eax], mm2        ; eax[21] = __r7g7b7
			add eax, 24              ; puc_out += 24
					
			inc edi
			jne horiz_loop1			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms
					
		}
		puc_out += _stride_out; 
        puc_y   += stride_y;
        _asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
horiz_loop2:
			
			movd mm2, [ecx]
			pxor mm7, mm7
			
			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			
			psubusb mm0, mmb_0x10    ; mm0 -= 16
			
			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0
			
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8
			
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8
			
			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8
			
			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v
			
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u
			
			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part
			
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0
			
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0
			
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1
			
			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma
			
			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0
			
			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1
			
			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1
			
			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1
			
			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0
			
			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpcklbw mm1, mm0       ; mm1 = __g3__g2__g1__g0
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpcklbw mm0, mm7       ; mm0 = r3b3r2b2r1b1r0b0
			
			movq mm2, mm0            ; mm2 = r3b3r2b2r1b1r0b0
			
			punpcklbw mm0, mm1       ; mm0 = __r1g1b1__r0g0b0
			punpckhbw mm2, mm1       ; mm2 = __r3g3b3__r2g2b2
			
			/* 24-bit shuffle and save... */
			movd   [eax], mm0        ; eax[0] = __r0g0b0
			psrlq mm0, 32            ; mm0 = __r1g1b1
			
			movd  3[eax], mm0        ; eax[3] = __r1g1b1
			
			movd  6[eax], mm2        ; eax[6] = __r2g2b2
			
			
			psrlq mm2, 32            ; mm2 = __r3g3b3
			
			movd  9[eax], mm2        ; eax[9] = __r3g3b3
			
			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?
			
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpckhbw mm1, mm0       ; mm1 = __g7__g6__g5__g4
			
			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpckhbw mm0, mm7       ; mm0 = r7b7r6b6r5b5r4b4
			
			movq mm2, mm0            ; mm2 = r7b7r6b6r5b5r4b4
			
			punpcklbw mm0, mm1       ; mm0 = __r5g5b5__r4g4b4
			punpckhbw mm2, mm1       ; mm2 = __r7g7b7__r6g6b6
			
			/* 24-bit shuffle and save... */
			movd 12[eax], mm0        ; eax[12] = __r4g4b4
			psrlq mm0, 32            ; mm0 = __r5g5b5
			
			movd 15[eax], mm0        ; eax[15] = __r5g5b5
			add ebx, 8               ; puc_y   += 8;
			
			movd 18[eax], mm2        ; eax[18] = __r6g6b6
			psrlq mm2, 32            ; mm2 = __r7g7b7
				
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;
			
			movd 21[eax], mm2        ; eax[21] = __r7g7b7
			add eax, 24              ; puc_out += 24
			
			inc edi
			jne horiz_loop2			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms	
		} 

		puc_out += _stride_out; 
        puc_y   += stride_y;
		puc_u   += stride_uv;
		puc_v   += stride_uv;
	}
}

// YUV -> RGB conversion, 16-bit output (two flavours)
// all stride values are in _bytes_
void CDirectXDraw::YUVToRGB555(BYTE *puc_y, int stride_y, 
								 BYTE *puc_u, BYTE *puc_v, int stride_uv, 
								 BYTE *puc_out,	int width_y, int height_y,
								 unsigned int _stride_out) 
{
    int y, horiz_count;
	
	horiz_count = -(width_y >> 3);
	
	for (y=0; y<height_y; y+=2) 
	{
		
		_asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
				
horiz_loop1:
			
			// load data
			movd mm2, [ecx]					 ; mm2 = ________u3u2u1u0
			movd mm3, [edx]					 ; mm3 = ________v3v2v1v0
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			
			pxor mm7, mm7						 ; zero mm7
			
			// convert chroma part
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			psubw mm2, mmw_0x0080    ; mm2 -= 128
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm2, 3             ; mm2 *= 8
			psllw mm3, 3             ; mm3 *= 8
			movq mm4, mm2            ; mm4 = mm2 = u
			movq mm5, mm3            ; mm5 = mm3 = v
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			paddsw mm2, mm3					 ; mm2 = green chroma
			
			// convert luma part
			psubusb mm0, mmb_0x10    ; mm0 -= 16
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0 luma even
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1 luma odd
			psllw mm0, 3             ; mm0 *= 8
			psllw mm1, 3             ; mm1 *= 8
			pmulhw mm0, mmw_mult_Y   ; mm0 luma odd *= luma coeff 
			pmulhw mm1, mmw_mult_Y   ; mm1 luma even *= luma coeff 
			
			// complete the matrix calc with the addictions
			movq mm3, mm4						 ; copy blue chroma
			movq mm6, mm5						 ; copy red chroma
			movq mm7, mm2						 ; copy green chroma
			paddsw mm3, mm0					 ; mm3 = luma odd + blue chroma
			paddsw mm4, mm1					 ; mm4 = luma even + blue chroma
			paddsw mm6, mm0					 ; mm6 = luma odd + red chroma
			paddsw mm5, mm1					 ; mm5 = luma even + red chroma
			paddsw mm7, mm0					 ; mm7 = luma odd + green chroma
			paddsw mm2, mm1					 ; mm2 = luma even + green chroma
			// clipping
			packuswb mm3, mm3
			packuswb mm4, mm4
			packuswb mm6, mm6
			packuswb mm5, mm5
			packuswb mm7, mm7
			packuswb mm2, mm2
			// interleave odd and even parts
			punpcklbw mm4, mm3			 ; mm4 = b7b6b5b4b3b2b1b0 blue
			punpcklbw mm5, mm6			 ; mm5 = r7r6r5r4r3r2r1r0 red
			punpcklbw mm2, mm7			 ; mm2 = g7g6g5g4g3g2g1g0 green
			
			// mask not needed bits (using 555)
			pand mm4, mask_5
			pand mm5, mask_5
			pand mm2, mask_5
			
			// mix colors and write
			
			psrlw mm4, 3						 ; mm4 = blue shifted
			pand mm4, mask_blue			 ; mask the blue again
			pxor mm7, mm7						 ; zero mm7
			movq mm1, mm4						 ; mm1 = copy blue
			movq mm3, mm5						 ; mm3 = copy red
			movq mm6, mm2						 ; mm6 = copy green
			
			punpckhbw mm1, mm7
			punpckhbw mm3, mm7
			punpckhbw mm6, mm7
			psllw mm6, 2						 ; shift green
			psllw mm3, 7						 ; shift red
			por mm6, mm3
			por mm6, mm1
			movq 8[eax], mm6
			
			punpcklbw mm2, mm7			 ; mm2 = __g3__g2__g1__g0 already masked
			punpcklbw mm5, mm7
			punpcklbw mm4, mm7
			psllw mm2, 2						 ; shift green
			psllw mm5, 7						 ; shift red
			por mm2, mm5
			por mm2, mm4
			movq [eax], mm2
			
			add ebx, 8               ; puc_y   += 8;
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;
			add eax, 16              ; puc_out += 16 // wrote 16 bytes
			
			inc edi
			jne horiz_loop1			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms
			
		}
		puc_out += _stride_out;
		puc_y   += stride_y;
        _asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
				
horiz_loop2:
			
			// load data
			movd mm2, [ecx]					 ; mm2 = ________u3u2u1u0
			movd mm3, [edx]					 ; mm3 = ________v3v2v1v0
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			
			pxor mm7, mm7						 ; zero mm7
			
			// convert chroma part
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			psubw mm2, mmw_0x0080    ; mm2 -= 128
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm2, 3             ; mm2 *= 8
			psllw mm3, 3             ; mm3 *= 8
			movq mm4, mm2            ; mm4 = mm2 = u
			movq mm5, mm3            ; mm5 = mm3 = v
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			paddsw mm2, mm3					 ; mm2 = green chroma
			
			// convert luma part
			psubusb mm0, mmb_0x10    ; mm0 -= 16
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0 luma even
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1 luma odd
			psllw mm0, 3             ; mm0 *= 8
			psllw mm1, 3             ; mm1 *= 8
			pmulhw mm0, mmw_mult_Y   ; mm0 luma odd *= luma coeff 
			pmulhw mm1, mmw_mult_Y   ; mm1 luma even *= luma coeff 
			
			// complete the matrix calc with the addictions
			movq mm3, mm4						 ; copy blue chroma
			movq mm6, mm5						 ; copy red chroma
			movq mm7, mm2						 ; copy green chroma
			paddsw mm3, mm0					 ; mm3 = luma odd + blue chroma
			paddsw mm4, mm1					 ; mm4 = luma even + blue chroma
			paddsw mm6, mm0					 ; mm6 = luma odd + red chroma
			paddsw mm5, mm1					 ; mm5 = luma even + red chroma
			paddsw mm7, mm0					 ; mm7 = luma odd + green chroma
			paddsw mm2, mm1					 ; mm2 = luma even + green chroma
			// clipping
			packuswb mm3, mm3
			packuswb mm4, mm4
			packuswb mm6, mm6
			packuswb mm5, mm5
			packuswb mm7, mm7
			packuswb mm2, mm2
			// interleave odd and even parts
			punpcklbw mm4, mm3			 ; mm4 = b7b6b5b4b3b2b1b0 blue
			punpcklbw mm5, mm6			 ; mm5 = r7r6r5r4r3r2r1r0 red
			punpcklbw mm2, mm7			 ; mm2 = g7g6g5g4g3g2g1g0 green
			
			// mask not needed bits (using 555)
			pand mm4, mask_5
			pand mm5, mask_5
			pand mm2, mask_5
			
			// mix colors and write
			
			psrlw mm4, 3						 ; mm4 = blue shifted
			pand mm4, mask_blue			 ; mask the blue again
			pxor mm7, mm7						 ; zero mm7
			movq mm1, mm4						 ; mm1 = copy blue
			movq mm3, mm5						 ; mm3 = copy red
			movq mm6, mm2						 ; mm6 = copy green
			
			punpckhbw mm1, mm7
			punpckhbw mm3, mm7
			punpckhbw mm6, mm7
			psllw mm6, 2						 ; shift green
			psllw mm3, 7						 ; shift red
			por mm6, mm3
			por mm6, mm1
			movq 8[eax], mm6
			
			punpcklbw mm2, mm7			 ; mm2 = __g3__g2__g1__g0 already masked
			punpcklbw mm5, mm7
			punpcklbw mm4, mm7
			psllw mm2, 2						 ; shift green
			psllw mm5, 7						 ; shift red
			por mm2, mm5
			por mm2, mm4
			movq [eax], mm2
			
			add ebx, 8               ; puc_y   += 8;
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;
			add eax, 16              ; puc_out += 16 // wrote 16 bytes
			
			inc edi
			jne horiz_loop2			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms
			
		}

		puc_out += _stride_out;
		puc_y   += stride_y;
		puc_u   += stride_uv;
		puc_v   += stride_uv;
	}
}

void CDirectXDraw::YUVToRGB565(BYTE *puc_y, int stride_y, 
								 BYTE *puc_u, BYTE *puc_v, int stride_uv, 
								 BYTE *puc_out, int width_y, int height_y,
								 unsigned int _stride_out) 
{
    
	int y, horiz_count;
	unsigned short * pus_out;
	pus_out = (unsigned short *) puc_out;
	horiz_count = -(width_y >> 3);
	
    _asm
	{
		
	}
	
	for (y=0; y<height_y; y+=2) 
	{
		_asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
				
horiz_loop1:
			
			// load data
			movd mm2, [ecx]					 ; mm2 = ________u3u2u1u0
			movd mm3, [edx]					 ; mm3 = ________v3v2v1v0
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			
			pxor mm7, mm7			 ; zero mm7
			
			// convert chroma part
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			psubw mm2, mmw_0x0080    ; mm2 -= 128  //0x0080008000800080
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm2, 3             ; mm2 *= 8    // u
			psllw mm3, 3             ; mm3 *= 8    // v
			movq mm4, mm2            ; mm4 = mm2 = u
			movq mm5, mm3            ; mm5 = mm3 = v
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff  //mmw_mult_U_G=0xf36ef36ef36ef36e;
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  //mmw_mult_V_G=0xe5e2e5e2e5e2e5e2;
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma     //mmw_mult_U_B=0x40cf40cf40cf40cf;  
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma      //mmw_mult_V_R=0x3343334333433343;
			paddsw mm2, mm3			 ; mm2 = green chroma    // u+v
			
			// convert luma part
			psubusb mm0, mmb_0x10    ; mm0 -= 16     ;y-16        //mmb_0x10=0x1010101010101010;
			
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0 luma even
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1 luma odd
			
			psllw mm0, 3             ; mm0 *= 8
			psllw mm1, 3             ; mm1 *= 8
			pmulhw mm0, mmw_mult_Y   ; mm0 luma odd *= luma coeff   //mmw_mult_Y= 0x2568256825682568;
			pmulhw mm1, mmw_mult_Y   ; mm1 luma even *= luma coeff  //
			
			// complete the matrix calc with the addictions
			movq mm3, mm4						 ; copy blue chroma
			movq mm6, mm5						 ; copy red chroma
			movq mm7, mm2						 ; copy green chroma
			paddsw mm3, mm0					 ; mm3 = luma odd + blue chroma
			paddsw mm4, mm1					 ; mm4 = luma even + blue chroma
			paddsw mm6, mm0					 ; mm6 = luma odd + red chroma
			paddsw mm5, mm1					 ; mm5 = luma even + red chroma
			paddsw mm7, mm0					 ; mm7 = luma odd + green chroma
			paddsw mm2, mm1					 ; mm2 = luma even + green chroma
			// clipping
			packuswb mm3, mm3
			packuswb mm4, mm4
			packuswb mm6, mm6
			packuswb mm5, mm5
			packuswb mm7, mm7
			packuswb mm2, mm2
			// interleave odd and even parts
			punpcklbw mm4, mm3			 ; mm4 = b7b6b5b4b3b2b1b0 blue
			punpcklbw mm5, mm6			 ; mm5 = r7r6r5r4r3r2r1r0 red
			punpcklbw mm2, mm7			 ; mm2 = g7g6g5g4g3g2g1g0 green
			
			// mask not needed bits (using 555)
			pand mm4, mask_5
			pand mm5, mask_5
			pand mm2, mask_5
			
			// mix colors and write
			
			psrlw mm4, 3						 ; mm4 = red shifted
			pand mm4, mask_blue			 ; mask the blue again
			pxor mm7, mm7						 ; zero mm7
			movq mm1, mm5						 ; mm1 = copy blue
			movq mm3, mm4						 ; mm3 = copy red
			movq mm6, mm2						 ; mm6 = copy green
			
			punpckhbw mm1, mm7
			punpckhbw mm3, mm7
			punpckhbw mm6, mm7
			psllw mm6, 3						 ; shift green
			psllw mm1, 8						 ; shift blue
			por mm6, mm3
			por mm6, mm1
			movq 8[eax], mm6
			
			punpcklbw mm2, mm7			 ; mm2 = __g3__g2__g1__g0 already masked
			punpcklbw mm4, mm7
			punpcklbw mm5, mm7
			psllw mm2, 3						 ; shift green
			psllw mm5, 8						 ; shift blue
			por mm2, mm4
			por mm2, mm5
			movq [eax], mm2
			
			add ebx, 8               ; puc_y   += 8;
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;
			add eax, 16              ; puc_out += 16 // wrote 16 bytes
			
			inc edi
			jne horiz_loop1			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms
		}
		puc_y   += stride_y;
		puc_out += _stride_out;
        _asm 
		{
			push eax
			push ebx
			push ecx
			push edx
			push edi
			
			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
			
horiz_loop2:
			
			// load data
			movd mm2, [ecx]					 ; mm2 = ________u3u2u1u0
			movd mm3, [edx]					 ; mm3 = ________v3v2v1v0
			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			
			pxor mm7, mm7			 ; zero mm7
			
			// convert chroma part
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0
			psubw mm2, mmw_0x0080    ; mm2 -= 128  //0x0080008000800080
			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm2, 3             ; mm2 *= 8    // u
			psllw mm3, 3             ; mm3 *= 8    // v
			movq mm4, mm2            ; mm4 = mm2 = u
			movq mm5, mm3            ; mm5 = mm3 = v
			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff  //mmw_mult_U_G=0xf36ef36ef36ef36e;
			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  //mmw_mult_V_G=0xe5e2e5e2e5e2e5e2;
			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma     //mmw_mult_U_B=0x40cf40cf40cf40cf;  
			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma      //mmw_mult_V_R=0x3343334333433343;
			paddsw mm2, mm3			 ; mm2 = green chroma    // u+v
			
			// convert luma part
			psubusb mm0, mmb_0x10    ; mm0 -= 16     ;y-16        //mmb_0x10=0x1010101010101010;
			
			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0 luma even
			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1 luma odd
			
			psllw mm0, 3             ; mm0 *= 8
			psllw mm1, 3             ; mm1 *= 8
			pmulhw mm0, mmw_mult_Y   ; mm0 luma odd *= luma coeff   //mmw_mult_Y= 0x2568256825682568;
			pmulhw mm1, mmw_mult_Y   ; mm1 luma even *= luma coeff  //
			
			// complete the matrix calc with the addictions
			movq mm3, mm4						 ; copy blue chroma
			movq mm6, mm5						 ; copy red chroma
			movq mm7, mm2						 ; copy green chroma
			paddsw mm3, mm0					 ; mm3 = luma odd + blue chroma
			paddsw mm4, mm1					 ; mm4 = luma even + blue chroma
			paddsw mm6, mm0					 ; mm6 = luma odd + red chroma
			paddsw mm5, mm1					 ; mm5 = luma even + red chroma
			paddsw mm7, mm0					 ; mm7 = luma odd + green chroma
			paddsw mm2, mm1					 ; mm2 = luma even + green chroma
			// clipping
			packuswb mm3, mm3
			packuswb mm4, mm4
			packuswb mm6, mm6
			packuswb mm5, mm5
			packuswb mm7, mm7
			packuswb mm2, mm2
			// interleave odd and even parts
			punpcklbw mm4, mm3			 ; mm4 = b7b6b5b4b3b2b1b0 blue
			punpcklbw mm5, mm6			 ; mm5 = r7r6r5r4r3r2r1r0 red
			punpcklbw mm2, mm7			 ; mm2 = g7g6g5g4g3g2g1g0 green
			
			// mask not needed bits (using 555)
			pand mm4, mask_5
			pand mm5, mask_5
			pand mm2, mask_5
			
			// mix colors and write
			
			psrlw mm4, 3						 ; mm4 = red shifted
			pand mm4, mask_blue			 ; mask the blue again
			pxor mm7, mm7						 ; zero mm7
			movq mm1, mm5						 ; mm1 = copy blue
			movq mm3, mm4						 ; mm3 = copy red
			movq mm6, mm2						 ; mm6 = copy green
			
			punpckhbw mm1, mm7
			punpckhbw mm3, mm7
			punpckhbw mm6, mm7
			psllw mm6, 3						 ; shift green
			psllw mm1, 8						 ; shift blue
			por mm6, mm3
			por mm6, mm1
			movq 8[eax], mm6
			
			punpcklbw mm2, mm7			 ; mm2 = __g3__g2__g1__g0 already masked
			punpcklbw mm4, mm7
			punpcklbw mm5, mm7
			psllw mm2, 3						 ; shift green
			psllw mm5, 8						 ; shift blue
			por mm2, mm4
			por mm2, mm5
			movq [eax], mm2
			
			add ebx, 8               ; puc_y   += 8;
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;
			add eax, 16              ; puc_out += 16 // wrote 16 bytes
			
			inc edi
			jne horiz_loop2			
			
			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax
			
			emms	
		}

		puc_y   += stride_y;
		puc_out += _stride_out;
		puc_u   += stride_uv;
		puc_v   += stride_uv;
	}
}

void CDirectXDraw::YUV2(	 BYTE* lpSrcY, 
							 int SrcPitch,
							 BYTE* lpSrcU,
							 BYTE* lpSrcV, 
							 int stride_uv, 
							 BYTE* lpDst, 
							 int SrcWidth, 
							 int SrcHeight,
							 unsigned int DstPitch )
								
{
	int SrcStride = SrcPitch + SrcPitch - SrcWidth;	
	int SrcStrideU = (SrcPitch - SrcWidth)>>1;
	int DstStride =(DstPitch - SrcWidth)<<1;

	__asm
	{
		push ebx
			
		mov edi , [lpDst]
		mov esi , [lpSrcY]
		mov eax , [lpSrcU]
		mov ebx , [lpSrcV]
		mov ecx , [SrcHeight]
		mov edx , [SrcWidth]
cyc: 
		movq mm2 , qword ptr [eax] //u
		movq mm3 , qword ptr [ebx] //v
		
		movq mm0 , qword ptr [esi] //y1
		movq mm1 , qword ptr [esi+8] //y2
		
		movq mm4 , mm2
		punpcklbw mm2 , mm3 // uv1
		punpckhbw mm4 , mm3 // uv2
				
		movq mm3 , mm0
		movq mm5 , mm1
		punpcklbw mm0 , mm2 // yuyv1
		punpckhbw mm3 , mm2 // yuyv2
		punpcklbw mm1 , mm4 // yuyv3
		punpckhbw mm5 , mm4 // yuyv4
		
		movq qword ptr [edi] , mm0
		movq qword ptr [edi+8] , mm3
		movq qword ptr [edi+16] , mm1
		movq qword ptr [edi+24] , mm5
		
		add esi , [SrcPitch]
		add edi , [DstPitch]
		
		movq mm0 , qword ptr [esi] //y1
		movq mm1 , qword ptr [esi+8] //y2
		
		movq mm3 , mm0
		movq mm5 , mm1
		punpcklbw mm0 , mm2 // yuyv1
		punpcklbw mm1 , mm4 // yuyv3
		punpckhbw mm3 , mm2 // yuyv2
		punpckhbw mm5 , mm4 // yuyv4
		
		movq qword ptr [edi] , mm0
		movq qword ptr [edi+8] , mm3
		movq qword ptr [edi+16] , mm1
		movq qword ptr [edi+24] , mm5
		
		sub esi , [SrcPitch]
		sub edi , [DstPitch]
		
		add eax , 8
		add ebx , 8
		add esi , 16
		add edi , 32
		
		sub edx,16			
		ja cyc
		
		mov edx,[SrcWidth]
		
		add esi , [SrcStride]
		add eax , [SrcStrideU]
		add ebx , [SrcStrideU]
		add edi , [DstStride]
		
		sub ecx,2
		ja cyc
		emms
		
		pop ebx
	}
}

void CDirectXDraw::ShowOSD(int nShowOSD)
{
	if (nShowOSD > 0)
	{
		m_bShowOSD = TRUE;
	}
	else
	{
		m_bShowOSD = FALSE;
	}
}

void CDirectXDraw::SetOSD(char *szOSD, int nTextColor, int nBackColor, int nFontSize, int nX, int nY)
{
	strcpy(m_szOSDText, szOSD);
	m_nTextColor = nTextColor;
	m_nBackColor = nBackColor;
	m_nFontSize = nFontSize;
	m_nX = nX;
	m_nY = nY;
}

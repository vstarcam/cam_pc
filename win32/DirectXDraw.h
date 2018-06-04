// **************************************************************
// File		   : DIRECTXDraw.h
// Description : 视频显示类
// Author	   :  
// Date		   : 
// Revisions   :
// **************************************************************
#ifndef _DIRECTX_DRAW_H
#define _DIRECTX_DRAW_H

#include <ddraw.h>

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"ddraw.lib")

typedef enum
{
	VIDEO_MODE_NONE,
	VIDEO_MODE_RGB555,
	VIDEO_MODE_RGB565,
	VIDEO_MODE_RGB24,
	VIDEO_MODE_RGB32,
	VIDEO_MODE_YUV2,
} Media_Video_Mode_E;


class CDirectXDraw
{
public:	
	CDirectXDraw(int nPlayHandle);
	~CDirectXDraw();
	
	//					初始化DirectDraw
	BOOL				InitDirectDraw(HWND hWndVideo,UINT nVideoWidth,UINT nVideoHeight,BOOL bUseYUVDraw = TRUE);

	//					显示YUV420视频
	BOOL				DrawImageFromYUVBuf(unsigned char *in_Y,
											unsigned char *in_U,
											unsigned char *in_V,
											unsigned int iWidth,
											unsigned int iHeight);


	//					设置视频偏移区域
	void				SetOffSetRect(CRect rcOffset);

	BYTE *CDirectXDraw::GetBufferPtr();

	void SnapShot(char *szPath);
	void ShowOSD(int nShowOSD);
	void SetOSD(char *szOSD, int nTextColor, int nBackColor, int nFontSize, int nX, int nY);
	void DrawOSD();
	void DisplayVideoWithRect(BOOL bDisplay, int nX, int nY, int nWidth, int nHeight);

protected:	
	void				ReleaseObject();
	void				UpdateBounds();
	BOOL				SetDisplayClipper(HWND hWndVideo);
	
	BOOL				CreateSourceYUV(HWND hWndVideo,UINT nVideoWidth,UINT nVideoHeight);	
	BOOL				CreateSourceRGB(HWND hWndVideo,UINT nVideoWidth,UINT nVideoHeight);
	
	void				YUVToRGB24(BYTE *puc_y, int stride_y, 
									BYTE *puc_u, BYTE *puc_v, int stride_uv, 
									BYTE *puc_out, int width_y, int height_y,
									unsigned int _stride_out);
							
	void				YUVToRGB32(BYTE *puc_y, int stride_y, 
									BYTE *puc_u, BYTE *puc_v, int stride_uv, 
									BYTE *puc_out, int width_y, int height_y,
									unsigned int _stride_out);

	void				YUVToRGB555(BYTE *puc_y, int stride_y, 
									BYTE *puc_u, BYTE *puc_v, int stride_uv, 
									BYTE *puc_out,	int width_y, int height_y,
									unsigned int _stride_out);

	void				YUVToRGB565(BYTE *puc_y, int stride_y, 
									BYTE *puc_u, BYTE *puc_v, int stride_uv, 
									BYTE *puc_out, int width_y, int height_y,
									unsigned int _stride_out);

	void				YUV2(BYTE *puc_y, int stride_y, 
									BYTE *puc_u, BYTE *puc_v, int stride_uv, 
									BYTE *puc_out, int width_y, int height_y,
									unsigned int _stride_out);

	void				YUVTOBMP24(BYTE *puc_y, int stride_y, 
									 BYTE *puc_u, BYTE *puc_v, int stride_uv, 
									 BYTE *puc_out, int width_y, int height_y,
									 unsigned int _stride_out);
	
	void				InitYUV2RGBTable();

	void				YUY2ToRGB24(const unsigned char* pSrc, DWORD dwSrc,unsigned char* pDst, DWORD dwDst);
		
	void				UYVY2YUYV(LPBYTE pBuf,int cbSize);	

	
		
protected:	
	HWND					m_hWndVideo;
	CRect					m_rcVideo;
	CRect					m_rcOffset;					// 偏移区域


	UINT					m_nVideoWidth;
	UINT					m_nVideoHeight;
	BOOL					m_bBltStretchX; 
	BOOL			        m_bAutoCheck;
	BOOL					m_bUseYUVDraw;

	Media_Video_Mode_E		m_emVideoMode;
	
	LPDIRECTDRAW7			m_pDD;
	LPDIRECTDRAWSURFACE7	m_pddsFrontBuffer;
	LPDIRECTDRAWSURFACE7	m_pddsBackBuffer;

	BOOL					m_bDrawImage;
	BOOL					m_bSafeExit;

private:	
	//void CapturePicture(int iWidth, int iHeight);
	BOOL CreateSnapShotDirectory(char *szPath);

	char m_szSnapShotPath[MAX_PATH];
	BOOL m_bSnapShot;

	int m_nPlayHandle;

	char m_szOSDText[128];
	int m_nTextColor;
	int m_nBackColor;
	BOOL m_bShowOSD;
	int m_nFontSize;
	int m_nX;
	int m_nY;

	BOOL m_bDisplayVideoWidthRect;
	int m_nVideoRectX;
	int m_nVideoRectY;
	int m_nVideoRectWidth;
	int m_nVideoRectHeight;
	
};

#endif			// _DIRECTX_DRAW_H


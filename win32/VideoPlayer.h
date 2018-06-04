#include "DirectXDraw.h"
#include "VideoBuf.h"
#include "VideoDecoder.h"

#ifndef _H264_PLAY_VIDEO_H_
#define _H264_PLAY_VIDEO_H_

#define GLB_WIDTH  1920
#define GLB_HEIGHT 1080

class CVideoPlayer
{
public:
	CVideoPlayer(void);
	~CVideoPlayer(void);

	BOOL StartPlay(HWND hWndVideo,UINT nVideoWidth,UINT nVideoHeight,BOOL bUseYUVDraw, int nPlayHandle);

	static DWORD WINAPI PlayVideoThread(LPVOID lParam);
	void PlayVideoProcess();
	void SendOneFrame(char *pBuf, int Len, BOOL bIFrame);
	void SnapShot(char *szPath);

	void StopPlay();
	BOOL IsPlaying();
	BOOL GetVideoSize(int& nWdith, int&nHeight);

	BOOL Refresh();
	void ShowOSD(int nShowOSD);
	void SetOSD(char *szOSD, int nTextColor, int nBackColor, int nFontSize, int nX, int nY);
	BOOL Pause(BOOL bPause);
	void DisplayVideoWithRect(BOOL bDisplay, int nX, int nY, int nWidth, int nHeight);

private:
	CDirectXDraw *m_pDirectXDraw;
	HANDLE m_hPlayVideoThread;

	CVideoBuf *m_pVideoBuf;
	BOOL m_bPlayThreadRuning;


	unsigned char			*m_pszImageBuffer;		//YUV图像数据	
	unsigned char			*m_pszYUVBuffer;		//YUV图像数据	

	BOOL m_bIsPlaying;

	int m_nWidth;
	int m_nHeight;

	int m_nPlayHandle;

	CCriticalSection m_cs;

	BOOL m_bPause;
	BOOL m_bWaitForIFrame;

    CVideoDecoder *m_pVideoDecoder;

};

#endif
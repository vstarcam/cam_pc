#include "StdAfx.h"
#include "VideoPlayer.h"

CVideoPlayer::CVideoPlayer(void)
{
	m_pDirectXDraw = NULL;
	m_bPlayThreadRuning = FALSE;
	m_hPlayVideoThread = NULL;

	m_pszImageBuffer = new unsigned char[2 * GLB_WIDTH * GLB_HEIGHT];	
	ASSERT(m_pszImageBuffer);

	m_pszYUVBuffer = new unsigned char[2 * GLB_WIDTH * GLB_HEIGHT];	
	ASSERT(m_pszYUVBuffer);
	memset(m_pszYUVBuffer, 0, 2 * GLB_WIDTH * GLB_HEIGHT);

	m_bIsPlaying = FALSE;

	m_nHeight = 0;
	m_nWidth = 0;

	m_pVideoBuf = NULL;
	m_pVideoBuf = new CVideoBuf();
	ASSERT(m_pVideoBuf);

	m_bPause = FALSE;

	m_bWaitForIFrame = TRUE;

    m_pVideoDecoder = new CVideoDecoder();

}

CVideoPlayer::~CVideoPlayer(void)
{
	StopPlay();
	SAFE_DELETE(m_pVideoBuf);
	SAFE_DELETE(m_pszImageBuffer);
	SAFE_DELETE(m_pszYUVBuffer);
    SAFE_DELETE(m_pVideoDecoder);
}

BOOL CVideoPlayer::Pause(BOOL bPause)
{
	if (!m_bIsPlaying)
	{
		return FALSE;
	}

	if (!bPause)
	{
		m_bWaitForIFrame = TRUE;
	}
	m_bPause = bPause;

	return TRUE;
}

void CVideoPlayer::SetOSD(char *szOSD, int nTextColor, int nBackColor, int nFontSize, int nX, int nY)
{
	if (m_pDirectXDraw == NULL)
	{
		return;
	}

	m_pDirectXDraw->SetOSD(szOSD, nTextColor, nBackColor, nFontSize, nX, nY);
}

void CVideoPlayer::ShowOSD(int nShowOSD)
{
	if (m_pDirectXDraw == NULL)
	{
		return ;
	}

	m_pDirectXDraw->ShowOSD(nShowOSD);
}

void CVideoPlayer::DisplayVideoWithRect(BOOL bDisplay, int nX, int nY, int nWidth, int nHeight)
{
	if (m_pDirectXDraw == NULL)
	{
		return;
	}

	m_pDirectXDraw->DisplayVideoWithRect(bDisplay, nX, nY, nWidth, nHeight);
}

BOOL CVideoPlayer::Refresh()
{
	if (m_nWidth == 0 || m_nHeight == 0)
	{
		return FALSE;
	}

	if (m_pDirectXDraw == NULL || !m_bIsPlaying)
	{
		return FALSE;
	}

	unsigned int  width    = m_nWidth;
	unsigned int height   = m_nHeight;
	unsigned char *pY		= (unsigned char*)m_pszYUVBuffer;
	unsigned char *pU		= ((unsigned char*)m_pszYUVBuffer)+width*height;
	unsigned char *pV		= ((unsigned char*)m_pszYUVBuffer)+width*height*5/4;			
	
	m_pDirectXDraw->DrawImageFromYUVBuf(pY,pU,pV, width, height);	

	return TRUE;	

}

void CVideoPlayer::StopPlay()
{
	m_cs.Lock();

	m_bIsPlaying = FALSE;

	if (m_hPlayVideoThread)
	{
		m_bPlayThreadRuning = FALSE;
		DWORD dwWait = WaitForSingleObject(m_hPlayVideoThread, INFINITE);
		if (dwWait == WAIT_OBJECT_0)
		{
			CloseHandle(m_hPlayVideoThread);
			m_hPlayVideoThread = NULL;
		}

		SAFE_DELETE(m_pDirectXDraw);
		m_pVideoBuf->FreeAllFrame();		


		memset(m_pszImageBuffer, 0, 2 * GLB_WIDTH * GLB_HEIGHT);
		memset(m_pszYUVBuffer, 0, 2 * GLB_WIDTH * GLB_HEIGHT);
		m_nWidth = 0;
		m_nHeight = 0;

		m_bPause = FALSE;
		m_bWaitForIFrame = TRUE;
	}

	m_cs.Unlock();

	
}

BOOL CVideoPlayer::GetVideoSize(int& nWdith, int&nHeight)
{
	if (m_nWidth == 0 || m_nHeight == 0)
	{
		return FALSE;
	}

	nWdith = m_nWidth;
	nHeight = m_nHeight;
	
	return TRUE;
}

BOOL CVideoPlayer::IsPlaying()
{
	return m_bIsPlaying;
}

BOOL CVideoPlayer::StartPlay(HWND hWndVideo,UINT nVideoWidth,UINT nVideoHeight,BOOL bUseYUVDraw, int nPlayHandle)
{
	m_cs.Lock();

	if (!IsWindow(hWndVideo))
	{
		m_cs.Unlock();
		return FALSE;
	}

	m_nPlayHandle = nPlayHandle;
	

	m_pDirectXDraw = new CDirectXDraw(m_nPlayHandle);
	ASSERT(m_pDirectXDraw);

	if (!m_pDirectXDraw->InitDirectDraw(hWndVideo, nVideoWidth, nVideoHeight, bUseYUVDraw))
	{
		TRACE("InitDirectDraw failed!\n");
		SAFE_DELETE(m_pDirectXDraw);
		m_cs.Unlock();
		return FALSE;
	}	

	m_bPlayThreadRuning = TRUE;
	m_hPlayVideoThread = CreateThread(NULL, 0, PlayVideoThread, this, 0, NULL);
	ASSERT(m_hPlayVideoThread);

	m_bIsPlaying = TRUE;

	m_cs.Unlock();

	return TRUE;
}

DWORD WINAPI CVideoPlayer::PlayVideoThread(LPVOID lParam)
{
	CVideoPlayer *pPlay = (CVideoPlayer*)lParam;

	pPlay->PlayVideoProcess();

	return 0;
}

void CVideoPlayer::PlayVideoProcess()
{
	char *pBuf = NULL;
	int Len = 0;
	BOOL bIFrame = FALSE;

	while(m_bPlayThreadRuning)
	{	

		if (!m_pVideoBuf->GetOneFrame(pBuf, Len, bIFrame))
		{
            //TRACE("GetOneFrame failed..\n");
			Sleep(10);
			continue;
		}

		if (m_bWaitForIFrame && bIFrame)
		{
			m_bWaitForIFrame = FALSE;
		}

		if (m_bPause || m_bWaitForIFrame)
		{
			SAFE_DELETE(pBuf);
			continue;
		}

		//TRACE("GetOneFrame Success...pBuf: 0x%x Len: %d\n", pBuf, Len);

        int nWidth ,nHeight;
        if (m_pVideoDecoder->DecoderFrame(pBuf, Len, (char*)m_pszImageBuffer, &nWidth, &nHeight) > 0)
        {
            //保存当前YUV数据
			memcpy(m_pszYUVBuffer, m_pszImageBuffer, 2 * GLB_WIDTH * GLB_HEIGHT);

			m_nHeight = nWidth;
			m_nWidth = nHeight;

			unsigned char *pY		= (unsigned char*)m_pszImageBuffer;
			unsigned char *pU		= ((unsigned char*)m_pszImageBuffer)+nWidth*nHeight;
			unsigned char *pV		= ((unsigned char*)m_pszImageBuffer)+nWidth*nHeight*5/4;			
			
			m_pDirectXDraw->DrawImageFromYUVBuf(pY,pU,pV, nWidth, nHeight);	
        }

	//	TRACE("CH264PlayVideo::H264PlayVideoProcess.... SAFE_DELETE(pBuf) begin... \n");
	//	TRACE("pBuf: 0x%x\n", pBuf);
		SAFE_DELETE(pBuf);
	//	TRACE("CH264PlayVideo::H264PlayVideoProcess.... SAFE_DELETE(pBuf) end... \n");
 	}
}

void CVideoPlayer::SendOneFrame(char *pBuf, int Len, BOOL bIFrame)
{
//	TRACE("Call CH264PlayVideo::H264SendOneFrame..\n");
	if (m_pVideoBuf)
	{
//		TRACE("m_pVideoBuf->SendOneFrame begin...\n");
		m_pVideoBuf->SendOneFrame(pBuf, Len, bIFrame);
//		TRACE("m_pVideoBuf->SendOneFrame end...\n");
	}
}

void CVideoPlayer::SnapShot(char *szPath)
{
	if (m_pDirectXDraw)
	{
		m_pDirectXDraw->SnapShot(szPath);
	}
}
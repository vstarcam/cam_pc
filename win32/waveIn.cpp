
#include "stdafx.h"
#include "waveIn.h"

#include <math.h>
#pragma comment(lib,"Winmm")

DWORD CWaveIn::s_dwInstance = 0;

CWaveIn::CWaveIn()
{
	m_wChannel = 1;
	m_dwSample = 8000;
	m_wBit = 16;
	m_buf_number = 10;

	m_hIn = 0;
	m_bThreadStart = FALSE;
	m_bDevOpen = FALSE;
	m_bAllocBuffer = FALSE;
	m_bRecord = FALSE;

	m_pHdr = NULL;
	m_dwAudioInId = 0;
	s_dwInstance++;

	m_bSend = FALSE;
	m_SelectCard = 0;
	m_pSaveDataCallback = NULL;
	m_pContext = NULL;

}

CWaveIn::~CWaveIn()
{

}


BOOL CWaveIn::Ini(WaveDataCallback pFunction, void *pContext)
{
	if( !pFunction )
	{	
		return FALSE;
	}

	m_pSaveDataCallback = pFunction;
	m_pContext = pContext;

	if (!StartThread())           
	{
		return FALSE;
	}
	
	if (!OpenDev())
	{
		StopThread ();
	}
	
	if (!PerPareBuffer())
	{
		CloseDev ();
	}

	return TRUE;
}

BOOL CWaveIn::Release()
{
	CloseRecord ();	
	StopThread ();
	FreeBuffer();
	if (CloseDev())
	{
		
	}
	return TRUE;
}

BOOL CWaveIn::StartThread()
{
	if (m_bThreadStart)
	{
		return TRUE;
	}
	m_hAudioIn = CreateThread(0 , 0 , AudioInThreadProc , this , 0 , &m_dwAudioInId );
	if(!m_hAudioIn)
	{
		TRACE(" ###>>> 对讲监听线程启动失败!\n");
		return FALSE;
	}
	m_bThreadStart = TRUE;
	return TRUE;
}

BOOL CWaveIn::StopThread()
{
	if (!m_bThreadStart)
	{
		return TRUE;
	}

	if(m_hAudioIn)
	{
		int t = 50;
		DWORD ExitCode;
		BOOL bEnd = FALSE;
		PostThreadMessage(m_dwAudioInId , WM_QUIT , 0 , 0);
		while(t)
		{
			GetExitCodeThread(m_hAudioIn,&ExitCode);
			if(ExitCode != STILL_ACTIVE)
			{
				bEnd = TRUE;
				break;
			}
			else
			{
				Sleep(10);
			}
			t--;
		}
		if(!bEnd)
		{
			TerminateThread(m_hAudioIn,0);
		}
		
		CloseHandle(m_hAudioIn);
		m_hAudioIn = 0;
	}

	m_bThreadStart = FALSE;
	return TRUE;
}

BOOL CWaveIn::OpenDev()
{
	if (m_bDevOpen)
	{
		return TRUE;
	}

	
	MMRESULT result;

    result = waveInGetNumDevs(); 
	if (result == 0)
	{
        TRACE(" ###>>> No Sound Device\n");
		return FALSE;
	}

	for(unsigned int i = 0 ; i < result; i++)
	{
		if (waveInGetDevCaps (i, &m_WaveInDevCaps, sizeof(WAVEINCAPS)) != MMSYSERR_NOERROR)
		{
			TRACE(" ###>>> Cannot determine sound card capabilities !\n");
			return FALSE;
		}
		TRACE(" ###>>> Found Audio Card %d , Name %s.\n" , i ,m_WaveInDevCaps.szPname );
		TRACE(" ###>>> We will use the NO.1 card\n");

	}

	WAVEFORMATEX wfx;
	wfx.wFormatTag		= WAVE_FORMAT_PCM;
	wfx.nChannels		= m_wChannel;
	wfx.nSamplesPerSec	= m_dwSample;
	wfx.nAvgBytesPerSec = m_wChannel * m_dwSample * m_wBit / 8;
	wfx.nBlockAlign		= m_wBit * m_wChannel / 8;
	wfx.wBitsPerSample	= m_wBit;
	wfx.cbSize			= 0;
	m_mmr = waveInOpen(0, m_SelectCard, &wfx, 0, 0, WAVE_FORMAT_QUERY);
	if (m_mmr)
	{
		TRACE(" ###>>> waveInOpen_query error!\n");
		return FALSE;
	}

	m_mmr=waveInOpen(&m_hIn, m_SelectCard, &wfx, m_dwAudioInId, s_dwInstance, CALLBACK_THREAD);
	if (m_mmr)
	{
		TRACE(" ###>>> waveInOpen error!\n");
		return FALSE;
	}

	m_bDevOpen = TRUE;
	return TRUE;
}

BOOL CWaveIn::CloseDev()
{
	if (!m_bDevOpen)
	{
		return TRUE;
	}

	if(!m_hIn)
	{
		TRACE(" ###>>> Device hasn't opened 1!\n");
		return FALSE;
	}

	m_mmr = waveInClose(m_hIn);
	if(m_mmr)
	{
		TRACE(GetLastErrorString());
		m_hIn = 0;
		m_bDevOpen = FALSE;
		return FALSE;
	}
	m_hIn = 0;
	m_bDevOpen = FALSE;
	return TRUE;
}

BOOL CWaveIn::PerPareBuffer()
{
	if (m_bAllocBuffer)
	{
		return TRUE;
	}

    
	m_mmr = waveInReset(m_hIn);
	if(m_mmr)
	{
		TRACE(" ###>>> waveInReset error!\n");
		return FALSE;
	}

	m_pHdr = new WAVEHDR[m_buf_number];
	
	
	for(int i = 0 ; i < m_buf_number ; i++)
	{
		ZeroMemory(&m_pHdr[i], sizeof(WAVEHDR));
		m_pHdr[i].lpData = InputBuffer[i];
		m_pHdr[i].dwBufferLength = SIZE_AUDIO_FRAME;
		
		m_mmr = waveInPrepareHeader(m_hIn, &m_pHdr[i], sizeof(WAVEHDR));
		if(m_mmr)
		{
			TRACE(" ###>>> waveInPrepareHeader error :%s! \n" , GetLastErrorString());
			return FALSE;
		}
		
		m_mmr=waveInAddBuffer(m_hIn,&m_pHdr[i],sizeof(WAVEHDR));
		if(m_mmr)
		{
			TRACE(" ###>>> waveInAddBuffer error!\n");
			return FALSE;
		}
	}

	m_bAllocBuffer=TRUE;
	return TRUE;
}


BOOL CWaveIn::FreeBuffer()
{
	if (!m_bAllocBuffer)
	{
		return TRUE;
	}
	
	if(!m_pHdr)
	{
		return FALSE;
	}

	
	m_mmr = waveInReset(m_hIn);
	if(m_mmr)
	{
		TRACE(" ###>>> waveInReset error!\n");
		return FALSE;
	}

	for(int i = 0 ; i < m_buf_number ; i++)
	{
		m_mmr = waveInUnprepareHeader(m_hIn,&m_pHdr[i],sizeof(WAVEHDR));
		if(m_mmr)
		{
			Sleep(100);
			TRACE(" ###>>> waveInUnprepareHeader error :%s!\n",GetLastErrorString());
			continue;
		}	
		if(m_pHdr[i].lpData)
		{
			
			
		}
	}
	delete []m_pHdr;
	m_pHdr = NULL;

	m_bAllocBuffer = FALSE;
	return TRUE;
}


BOOL CWaveIn::OpenRecord()
{
	if (m_bRecord)
	{
		return TRUE;
	}

	if(!m_hIn)
	{
		TRACE(" ###>>> Device hasn't opened! 2\n");
		return FALSE;
	}
    
	m_mmr = waveInStart(m_hIn);
	if(m_mmr)
	{
		TRACE(" ###>>> waveInStart error!\n");
		return FALSE;
	}

	m_bRecord=TRUE;

#ifdef Aduio_Test
	m_AudioPlay.Open();
#endif
	return TRUE;
}


BOOL CWaveIn::CloseRecord()
{
	if (!m_bRecord)
	{
		return TRUE;
	}
	

	if(!m_hIn)
	{
		TRACE(" ###>>> Device hasn't opened! 3\n");
		return FALSE;
	}

	m_mmr = waveInStop(m_hIn);
	if(m_mmr)
	{
		TRACE(" ###>>> CWaveIn::CloseRecord: waveInStop error.\n");
		return FALSE;
	}
 	
	m_bRecord=FALSE;

#ifdef Aduio_Test
	m_AudioPlay.Close();
#endif

	return TRUE;
}

MMRESULT CWaveIn::GetLastMMError()
{
	return m_mmr;
}

CString CWaveIn::GetLastErrorString()
{
	
	TCHAR buffer[256];
	memset(buffer,0,256);
	waveInGetErrorText(m_mmr,buffer,256);
	return buffer;
}

void CWaveIn::SetBit(WORD wBit)
{
	m_wBit = (wBit == 8) ? 8:16;
}

void CWaveIn::SetSample(DWORD dwSample)
{
	m_dwSample = dwSample;
}

void CWaveIn::SetChannel(WORD wChannel)
{
	m_wChannel = (m_wChannel == wChannel) ? 2:1;
}

void CWaveIn::SetBuf_Number(int buf_number)
{
	m_buf_number=buf_number;
}

WORD CWaveIn::GetBit()
{
	return m_wBit;
}

DWORD CWaveIn::GetSample()
{
	return m_dwSample;
}

WORD CWaveIn::GetChannel()
{
	return m_wChannel;
}

int CWaveIn::GetBuf_Number()
{
	return m_buf_number;
}

DWORD CWaveIn::GetInstance()
{
	return s_dwInstance;
}

void CWaveIn::SaveData(char *pBuffer,int iLen)
{
	if(m_pSaveDataCallback)	
	{
		unsigned char szbuf[2500]={0};
		m_adpcm.ADPCMEncode((unsigned char *)pBuffer,iLen,szbuf);
		m_pSaveDataCallback((const char *)szbuf,iLen/4, m_pContext);

	#ifdef Aduio_Test
		m_AudioPlay.input((unsigned char*)pBuffer, iLen);
#endif

		





	}

}

void CWaveIn::EnableSend(BOOL bSend)
{
	InterlockedExchange(&m_bSend, bSend);
}

DWORD WINAPI CWaveIn::AudioInThreadProc(LPVOID lpParameter)
{
	char buffer[SIZE_AUDIO_FRAME];
	CWaveIn *pWaveIn = (CWaveIn *)lpParameter;
	CString s;
    
	MSG msg;
	while(GetMessage(&msg,0,0,0))
	{
		switch(msg.message )
		{
		case MM_WIM_OPEN:
			
			TRACE(" ###>>> RECV WIM OPEN message!\n");
			break;
		case MM_WIM_CLOSE:
			
			TRACE(" ###>>> RECV WIM CLOSE message!\n");
			break;
		case MM_WIM_DATA:
			
			
			
			WAVEHDR* pWH=(WAVEHDR*)msg.lParam;
			if(pWH->dwBytesRecorded!=SIZE_AUDIO_FRAME)
				break;

			waveInUnprepareHeader((HWAVEIN)msg.wParam,pWH,sizeof(WAVEHDR));
			
			
            
			memcpy(buffer,pWH->lpData,pWH->dwBytesRecorded);
			pWaveIn->SaveData (buffer ,pWH->dwBytesRecorded );
            
			waveInPrepareHeader((HWAVEIN)msg.wParam,pWH,sizeof(WAVEHDR));
			waveInAddBuffer((HWAVEIN)msg.wParam,pWH,sizeof(WAVEHDR));

			break;
		}
		Sleep(1);
	}
	return (DWORD)msg.wParam;
}


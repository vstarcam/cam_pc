#include "StdAfx.h"
#include "waveOut.h"

CWaveOut::CWaveOut()
{
	/*
	
	TWaveFormatEx = packed record
	wFormatTag: Word;       {ָ����ʽ����; Ĭ�� WAVE_FORMAT_PCM = 1;}
	nChannels: Word;        {ָ���������ݵ�ͨ����; ������Ϊ 1, ������Ϊ 2}
	nSamplesPerSec: DWORD;  {ָ����������(ÿ���������)}
	nAvgBytesPerSec: DWORD; {ָ�����ݴ����ƽ������(ÿ����ֽ���)}
	nBlockAlign: Word;      {ָ�������(��λ�ֽ�), ����������ݵ���С��λ}
	wBitsPerSample: Word;   {������С(�ֽ�)}
	cbSize: Word;           {������Ϣ��С; PCM ��ʽû����ֶ�}
	end;
	{16 λ������ PCM �Ŀ������ 4 �ֽ�(ÿ������2�ֽ�, 2��ͨ��)}
	*/
	m_waveFormat.wFormatTag		= WAVE_FORMAT_PCM;
	m_waveFormat.nChannels		= 1;
	m_waveFormat.nSamplesPerSec	= 8000;
	m_waveFormat.wBitsPerSample	= 16;
	m_waveFormat.nAvgBytesPerSec= 16000;
	m_waveFormat.nBlockAlign	= 2;
	m_waveFormat.cbSize			= 0;

	m_hWavOut					= NULL;
}

CWaveOut::~CWaveOut()
{
	Close();
}

void __stdcall mywaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == WOM_DONE)
	{
		WAVEHDR *pWavhdr = (WAVEHDR *)dwParam1;
		CWaveOut *pWvo = (CWaveOut *)dwInstance;
		pWvo->m_cs.Lock();
		pWvo->m_buf_free.push_back((char *)pWavhdr);
		pWvo->m_cs.Unlock();
	}
}

int CWaveOut::input(unsigned char* buf , int videoLen)
{
	size_t nSize = m_buf_free.size();
	if (nSize == 0)
	{
		TRACE("!!!~~~~~### %d - %d\n",videoLen,m_buf_free.size());
		return 0;
	}

	
	m_cs.Lock();
	WAVEHDR *pWavehdr = (WAVEHDR *)m_buf_free.front();
	m_buf_free.pop_front();
	m_cs.Unlock();


	memset(pWavehdr->lpData, MAX_AU_SIZE,0);
	memcpy(pWavehdr->lpData, buf, videoLen);
	waveOutWrite(m_hWavOut, pWavehdr, sizeof(WAVEHDR));



	return 0;
}
BOOL CWaveOut::Open()
{
	if (m_hWavOut != NULL)
	{
		return TRUE;
	}
	/*
	waveOutOpen(
	lphWaveOut: PHWaveOut;   {���ڷ����豸�����ָ��; ��� dwFlags=WAVE_FORMAT_QUERY, ����Ӧ�� nil}
	uDeviceID: UINT;         {�豸ID; ����ָ��Ϊ: WAVE_MAPPER, ������������ݸ����Ĳ��θ�ʽѡ����ʵ��豸}
	lpFormat: PWaveFormatEx; {TWaveFormat �ṹ��ָ��; TWaveFormat ����Ҫ����Ĳ��θ�ʽ}
	dwCallback: DWORD        {�ص�������ַ�򴰿ھ��; ����ʹ�ûص�����, ��Ϊ nil}
	dwInstance: DWORD        {���ص�������ʵ������; �����ڴ���}
	dwFlags: DWORD           {��ѡ��}
	)
	*/
	MMRESULT nRet = waveOutOpen(&m_hWavOut, WAVE_MAPPER, &m_waveFormat, (DWORD)mywaveOutProc, (DWORD) this, CALLBACK_FUNCTION);
	if (nRet != MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	for(int i=0; i<BUFFER_SIZE; ++i)
	{
		WAVEHDR *pWavhdr		= (WAVEHDR *)m_buf[i];
		pWavhdr->dwBufferLength = MAX_AU_SIZE ;
		pWavhdr->lpData			= m_buf[i] + sizeof(WAVEHDR);
		pWavhdr->dwFlags		= 0;

		waveOutPrepareHeader(m_hWavOut, pWavhdr, sizeof(WAVEHDR));
		m_buf_free.push_back((char *)pWavhdr);
	}


	m_nTemp = 0;
	return TRUE;
}

BOOL CWaveOut::Close()
{
	if (m_hWavOut != NULL)
	{
		while (m_buf_free.size() != BUFFER_SIZE)
			Sleep(80);
		m_buf_free.clear();
		waveOutPause(m_hWavOut);
		waveOutReset(m_hWavOut);

		for (int i=0; i<BUFFER_SIZE; ++i)
		{
			WAVEHDR *pWavhdr = (WAVEHDR *)m_buf[i];
			waveOutUnprepareHeader(m_hWavOut, pWavhdr, sizeof(WAVEHDR));
		}

		waveOutClose(m_hWavOut);
		m_hWavOut = NULL;
	}
		
	return TRUE;
}

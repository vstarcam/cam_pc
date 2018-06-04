#include "StdAfx.h"
#include "waveOut.h"

CWaveOut::CWaveOut()
{
	/*
	
	TWaveFormatEx = packed record
	wFormatTag: Word;       {指定格式类型; 默认 WAVE_FORMAT_PCM = 1;}
	nChannels: Word;        {指出波形数据的通道数; 单声道为 1, 立体声为 2}
	nSamplesPerSec: DWORD;  {指定样本速率(每秒的样本数)}
	nAvgBytesPerSec: DWORD; {指定数据传输的平均速率(每秒的字节数)}
	nBlockAlign: Word;      {指定块对齐(单位字节), 块对齐是数据的最小单位}
	wBitsPerSample: Word;   {采样大小(字节)}
	cbSize: Word;           {附加信息大小; PCM 格式没这个字段}
	end;
	{16 位立体声 PCM 的块对齐是 4 字节(每个样本2字节, 2个通道)}
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
	lphWaveOut: PHWaveOut;   {用于返回设备句柄的指针; 如果 dwFlags=WAVE_FORMAT_QUERY, 这里应是 nil}
	uDeviceID: UINT;         {设备ID; 可以指定为: WAVE_MAPPER, 这样函数会根据给定的波形格式选择合适的设备}
	lpFormat: PWaveFormatEx; {TWaveFormat 结构的指针; TWaveFormat 包含要申请的波形格式}
	dwCallback: DWORD        {回调函数地址或窗口句柄; 若不使用回调机制, 设为 nil}
	dwInstance: DWORD        {给回调函数的实例数据; 不用于窗口}
	dwFlags: DWORD           {打开选项}
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

#ifndef __WAVEIN_H__
#define __WAVEIN_H__

#include <mmsystem.h>
#include <afxmt.h>
#include "Adpcm.h"



#define SIZE_AUDIO_FRAME	(256*4)
#define SIZE_AUDIO_PACKED	(70 + 2)  
typedef BOOL (CALLBACK* WaveDataCallback)(const char *pData ,unsigned int nDataLen, void *pContext);





#ifdef Aduio_Test
#include "waveOut.h"
#endif



class CWaveIn  
{


public:
#ifdef Aduio_Test
	CWaveOut m_AudioPlay;
#endif
public:
	
	BOOL OpenRecord();
	BOOL CloseRecord();
	BOOL Ini(WaveDataCallback pFunction, void *pContext);


	CAdpcm m_adpcm;
public:
	

	BOOL Release();

	static DWORD WINAPI AudioInThreadProc(LPVOID lpParameter);

	
	WaveDataCallback	m_pSaveDataCallback;
	void				*m_pContext;

	
	MMRESULT GetLastMMError();
	CString GetLastErrorString();

	
	inline void SetBit(WORD wBit);
	inline void SetSample(DWORD dwSample);
	inline void SetChannel(WORD wChannel);
	inline void SetBuf_Number(int buf_number);

	
	inline WORD GetBit();
	inline DWORD GetSample();
	inline WORD GetChannel();
	inline int GetBuf_Number();
	inline DWORD GetInstance();
    
	
	virtual void SaveData(char *pBuffer,int iLen);
	void EnableSend(BOOL bSend);
	
	CWaveIn();
	virtual ~CWaveIn();	


	BOOL m_SelectCard;

protected:
	
	BOOL StopThread();
	BOOL StartThread();
	
	BOOL OpenDev();
	BOOL CloseDev();
	
	BOOL PerPareBuffer();
	BOOL FreeBuffer();
	


protected:

	
	WORD m_wChannel;
	
	DWORD m_dwSample;
	
	WORD m_wBit;
	
	int m_buf_number;

	static DWORD s_dwInstance;

    HWAVEIN m_hIn;
	MMRESULT m_mmr;
	DWORD m_dwAudioInId;

	HANDLE m_hAudioIn;
	WAVEHDR* m_pHdr;
	WAVEINCAPS m_WaveInDevCaps;

	BOOL m_bThreadStart;
	BOOL m_bDevOpen;
	BOOL m_bAllocBuffer;
	BOOL m_bRecord;
	LONG m_bSend;

	char InputBuffer[10][SIZE_AUDIO_FRAME];
	char OutputBuffer[SIZE_AUDIO_PACKED];
};

#endif 


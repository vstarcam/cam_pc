// TestP2PDlg.h : 头文件
//

#include "Adpcm.h"
#include "VideoPlayer.h"
#include "waveOut.h"
#include "waveIn.h"

#pragma once


// CTestP2PDlg 对话框
class CTestP2PDlg : public CDialog
{
// 构造
public:
	CTestP2PDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CTestP2PDlg();

// 对话框数据
	enum { IDD = IDD_TESTP2P_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnInitial();
    afx_msg void OnBnClickedBtnDeinitial();
    afx_msg void OnBnClickedBtnConnect();
    afx_msg void OnBnClickedBtnCreateInstance();

    static void AVDataCallBack(long nHandle, int bVideo ,char *pData, int len, void *pParam);
    static void MessageCallBack(long nHandle, int type, char *msg, int len, void *pParam);

	static DWORD WINAPI APlayWorkThread(LPVOID lpParam);
	void APlay();

    static DWORD WINAPI TalkThread(LPVOID lPARAM);
    void TalkProcess();
	static BOOL CALLBACK TalkDataCallBack(const char *pData , unsigned int nDataLen, void *pContext);

private:
    long m_hP2PHandle;
    CAdpcm *m_pAdpcm;
    BOOL m_bTalkThreadRuning;
    HANDLE m_hTalkThread;
    CVideoPlayer *m_pVideoPlayer;

	//Audio Play
	CWaveOut     m_WaveOut;	
	HANDLE		 m_hAPlayThread;
	BOOL         m_bAudioPlay;
	BOOL         m_bAudioIsFirst;
	CPtrList     m_FrameQueueAudio;
	CRITICAL_SECTION  m_secAudio;

	//Audio Talk
	CWaveIn      m_WaveIn;
	BOOL         m_bAudioTalk;

public:
    afx_msg void OnBnClickedBtnDestroyInstance2();
    afx_msg void OnBnClickedBtnSetAvdatacallback();
    afx_msg void OnBnClickedBtnSetMessageCallback();
    afx_msg void OnBnClickedBtnStartAudio();
    afx_msg void OnBnClickedBtnStopAudio();
    afx_msg void OnBnClickedBtnStartVideo();
    afx_msg void OnBnClickedBtnStopVideo();
    afx_msg void OnBnClickedBtnCloseP2p();
    afx_msg void OnBnClickedBtnDestroyAllInstance();
    afx_msg void OnBnClickedBtnCloseAll();
    afx_msg void OnBnClickedBtnGetCameraParams();
    afx_msg void OnBnClickedBtnDecoderControl();
    afx_msg void OnBnClickedBtnSnapshot();
    afx_msg void OnBnClickedBtnCameraControl();
    afx_msg void OnClose();
    afx_msg void OnBnClickedBtnSetNetwork();
    afx_msg void OnBnClickedBtnReboot();
    afx_msg void OnBnClickedBtnRestore();
    afx_msg void OnBnClickedBtnWifiScan();
//    afx_msg void OnBnClickedBtnWifiScanResult();
    afx_msg void OnBnClickedBtnGetParams();
    afx_msg void OnBnClickedBtnGetStatus();
    afx_msg void OnBnClickedBtnGetPtzParams();
    afx_msg void OnBnClickedBtnGetAlarmLog();
    afx_msg void OnBnClickedBtnStartTalk();
    afx_msg void OnBnClickedBtnStopTalk();
    afx_msg void OnBnClickedBtnDecoderControlLeft();
    afx_msg void OnBnClickedBtnDecoderControlRight();
    afx_msg void OnBnClickedBtnDecoderControlDown();
    afx_msg void OnBnClickedBtnSetDate();
    afx_msg void OnBnClickedBtnSetUser();
    afx_msg void OnBnClickedBtnSetMail();
    afx_msg void OnBnClickedBtnSetFtp();
    afx_msg void OnBnClickedBtnSetWifi();
    afx_msg void OnBnClickedBtnSetAlarm();
    afx_msg void OnBnClickedBtnSetDeviceName();
    afx_msg void OnBnClickedBtnSetPtz();
	afx_msg void OnBnClickedBtnSetpreset();
	afx_msg void OnBnClickedBtnGetpreset();
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};

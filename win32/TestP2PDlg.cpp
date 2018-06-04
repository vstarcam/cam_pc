// TestP2PDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TestP2P.h"
#include "TestP2PDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WIN32_DLL 1

#include "P2P_API.h"
#pragma comment(lib, "P2PAPI.lib")



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTestP2PDlg 对话框


CTestP2PDlg::CTestP2PDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestP2PDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_pAdpcm = NULL;
    m_bTalkThreadRuning = FALSE;
    m_hTalkThread = NULL;

	m_bAudioPlay   = FALSE;
	m_bAudioIsFirst= FALSE; 
	m_hAPlayThread = NULL;

	m_bAudioTalk   = FALSE;

	InitializeCriticalSection(&m_secAudio);
}

CTestP2PDlg::~CTestP2PDlg()
{
	DeleteCriticalSection(&m_secAudio);
}

void CTestP2PDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestP2PDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_INITIAL, &CTestP2PDlg::OnBnClickedBtnInitial)
    ON_BN_CLICKED(IDC_BTN_DEINITIAL, &CTestP2PDlg::OnBnClickedBtnDeinitial)
    ON_BN_CLICKED(IDC_BTN_CONNECT, &CTestP2PDlg::OnBnClickedBtnConnect)
    ON_BN_CLICKED(IDC_BTN_CREATE_INSTANCE, &CTestP2PDlg::OnBnClickedBtnCreateInstance)
    ON_BN_CLICKED(IDC_BTN_DESTROY_INSTANCE2, &CTestP2PDlg::OnBnClickedBtnDestroyInstance2)
    ON_BN_CLICKED(IDC_BTN_SET_AVDATACALLBACK, &CTestP2PDlg::OnBnClickedBtnSetAvdatacallback)
    ON_BN_CLICKED(IDC_BTN_SET_MESSAGE_CALLBACK, &CTestP2PDlg::OnBnClickedBtnSetMessageCallback)
    ON_BN_CLICKED(IDC_BTN_START_AUDIO, &CTestP2PDlg::OnBnClickedBtnStartAudio)
    ON_BN_CLICKED(IDC_BTN_STOP_AUDIO, &CTestP2PDlg::OnBnClickedBtnStopAudio)
    ON_BN_CLICKED(IDC_BTN_START_VIDEO, &CTestP2PDlg::OnBnClickedBtnStartVideo)
    ON_BN_CLICKED(IDC_BTN_STOP_VIDEO, &CTestP2PDlg::OnBnClickedBtnStopVideo)
    ON_BN_CLICKED(IDC_BTN_CLOSE_P2P, &CTestP2PDlg::OnBnClickedBtnCloseP2p)
    ON_BN_CLICKED(IDC_BTN_DESTROY_ALL_INSTANCE, &CTestP2PDlg::OnBnClickedBtnDestroyAllInstance)
    ON_BN_CLICKED(IDC_BTN_CLOSE_ALL, &CTestP2PDlg::OnBnClickedBtnCloseAll)
    ON_BN_CLICKED(IDC_BTN_GET_CAMERA_PARAMS, &CTestP2PDlg::OnBnClickedBtnGetCameraParams)
    ON_BN_CLICKED(IDC_BTN_DECODER_CONTROL, &CTestP2PDlg::OnBnClickedBtnDecoderControl)
    ON_BN_CLICKED(IDC_BTN_SNAPSHOT, &CTestP2PDlg::OnBnClickedBtnSnapshot)
    ON_BN_CLICKED(IDC_BTN_CAMERA_CONTROL, &CTestP2PDlg::OnBnClickedBtnCameraControl)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_SET_NETWORK, &CTestP2PDlg::OnBnClickedBtnSetNetwork)
    ON_BN_CLICKED(IDC_BTN_REBOOT, &CTestP2PDlg::OnBnClickedBtnReboot)
    ON_BN_CLICKED(IDC_BTN_RESTORE, &CTestP2PDlg::OnBnClickedBtnRestore)
    ON_BN_CLICKED(IDC_BTN_WIFI_SCAN, &CTestP2PDlg::OnBnClickedBtnWifiScan)
    //    ON_BN_CLICKED(IDC_BTN_WIFI_SCAN_RESULT, &CTestP2PDlg::OnBnClickedBtnWifiScanResult)
    ON_BN_CLICKED(IDC_BTN_GET_PARAMS, &CTestP2PDlg::OnBnClickedBtnGetParams)
    ON_BN_CLICKED(IDC_BTN_GET_STATUS, &CTestP2PDlg::OnBnClickedBtnGetStatus)
    ON_BN_CLICKED(IDC_BTN_GET_PTZ_PARAMS, &CTestP2PDlg::OnBnClickedBtnGetPtzParams)
    ON_BN_CLICKED(IDC_BTN_GET_ALARM_LOG, &CTestP2PDlg::OnBnClickedBtnGetAlarmLog)
    ON_BN_CLICKED(IDC_BTN_START_Talk, &CTestP2PDlg::OnBnClickedBtnStartTalk)
    ON_BN_CLICKED(IDC_BTN_STOP_TALK, &CTestP2PDlg::OnBnClickedBtnStopTalk)
    ON_BN_CLICKED(IDC_BTN_DECODER_CONTROL_LEFT, &CTestP2PDlg::OnBnClickedBtnDecoderControlLeft)
    ON_BN_CLICKED(IDC_BTN_DECODER_CONTROL_RIGHT, &CTestP2PDlg::OnBnClickedBtnDecoderControlRight)
    ON_BN_CLICKED(IDC_BTN_DECODER_CONTROL_DOWN, &CTestP2PDlg::OnBnClickedBtnDecoderControlDown)
    ON_BN_CLICKED(IDC_BTN_SET_DATE, &CTestP2PDlg::OnBnClickedBtnSetDate)
    ON_BN_CLICKED(IDC_BTN_SET_USER, &CTestP2PDlg::OnBnClickedBtnSetUser)
    ON_BN_CLICKED(IDC_BTN_SET_MAIL, &CTestP2PDlg::OnBnClickedBtnSetMail)
    ON_BN_CLICKED(IDC_BTN_SET_FTP, &CTestP2PDlg::OnBnClickedBtnSetFtp)
    ON_BN_CLICKED(IDC_BTN_SET_WIFI, &CTestP2PDlg::OnBnClickedBtnSetWifi)
    ON_BN_CLICKED(IDC_BTN_SET_ALARM, &CTestP2PDlg::OnBnClickedBtnSetAlarm)
    ON_BN_CLICKED(IDC_BTN_SET_DEVICE_NAME, &CTestP2PDlg::OnBnClickedBtnSetDeviceName)
    ON_BN_CLICKED(IDC_BTN_SET_PTZ, &CTestP2PDlg::OnBnClickedBtnSetPtz)
	ON_BN_CLICKED(IDC_BTN_SETPRESET, &CTestP2PDlg::OnBnClickedBtnSetpreset)
	ON_BN_CLICKED(IDC_BTN_GETPRESET, &CTestP2PDlg::OnBnClickedBtnGetpreset)
END_MESSAGE_MAP()


// CTestP2PDlg 消息处理程序

BOOL CTestP2PDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    m_pAdpcm = new CAdpcm();

    m_pVideoPlayer = new CVideoPlayer();

	//modify 20171214 by jerry
    //m_pVideoPlayer->StartPlay(GetDlgItem(IDC_PLAY_RECT)->m_hWnd, 1280, 720, FALSE, 0);
	m_pVideoPlayer->StartPlay(GetDlgItem(IDC_PLAY_RECT)->m_hWnd, GLB_WIDTH, GLB_HEIGHT, FALSE, 0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTestP2PDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestP2PDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTestP2PDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//1
void CTestP2PDlg::OnBnClickedBtnInitial()
{
    // TODO: 在此添加控件通知处理程序代码
//	 P2PAPI_Initial();

	//modify 20171214 by jerry
	//long nRet = P2PAPI_InitialWithServer("EFGBFFBJKDJKGGJMEAHKFHEJHNNHHANIHIFBBKDFAMJDLKKODNANCBPJGGLEIELOBGNDKADKPGNJBNCNIF"); //VSTA
	//long nRet = P2PAPI_InitialWithServer("EBGBEMBMKGJMGAJPEIGIFKEGHBMCHMNFGKEGBFCBBMJELILDCJADCIOLHHLLJBKEAMMBLCDGONMDBJCJJPNFJP"); //VSTC
	//long nRet = P2PAPI_InitialWithServer("EFGFFBBOKAIEGHJAEDHJFEEOHMNGDCNJCDFKAKHLEBJHKEKMCAFCDLLLHAOCJPPMBHMNOMCJKGJEBGGHJHIOMFBDNPKNFEGCEGCBGCALMFOHBCGMFK"); //PISR
	//long nRet = P2PAPI_InitialWithServer("HZLXEJIALKHYATPCHULNSVLMEELSHWIHPFIBAOHXIDICSQEHENEKPAARSTELERPDLNEPLKEILPHUHXHZEJEEEHEGEM-$$"); //VSTD
	long nRet = P2PAPI_InitialWithServer("HZLXEJIALKHYATPCHULNSVLMEELSHWIHPFIBAOHXIDICSQEHENEKPAARSTELERPDLNEPLKEILPHUHXHZEJEEEHEGEM-$$"); //VSTF
	DPrintf("\r\n---P2PAPI_InitialWithServer nRet=%d---", nRet);
}

void CTestP2PDlg::OnBnClickedBtnDeinitial()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_DeInitial();
}

//5
void CTestP2PDlg::OnBnClickedBtnConnect()
{
    // TODO: 在此添加控件通知处理程序代码
//	P2PAPI_Connect(m_hP2PHandle, "VSTC172299NEWGD", "admin", "123456",1);

	//modify 20171214 by jerry
	//long nRet = P2PAPI_Connect(m_hP2PHandle, "VSTC633639FWLSE", "admin", "888888",1);
	//long nRet = P2PAPI_Connect(m_hP2PHandle, "PISR016989HCUFG", "admin", "123123",1);
	//long nRet = P2PAPI_Connect(m_hP2PHandle, "VSTD169744QXHDI", "admin", "888888",1);
	long nRet = P2PAPI_Connect(m_hP2PHandle, "VSTF188912NLVSR", "admin", "888888",1);
	DPrintf("\r\n---P2PAPI_Connect nRet=%d---", nRet);
	
	//long h = P2PAPI_Connect(m_hP2PHandle, "gfjghjf", "admin", "");
}

//2
void CTestP2PDlg::OnBnClickedBtnCreateInstance()
{
    // TODO: 在此添加控件通知处理程序代码
    long nRet = P2PAPI_CreateInstance(&m_hP2PHandle);
	DPrintf("\r\n---P2PAPI_CreateInstance nRet=%d---", nRet);
}

void CTestP2PDlg::OnBnClickedBtnDestroyInstance2()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_DestroyInstance(m_hP2PHandle);
}

void CTestP2PDlg::AVDataCallBack(long nHandle, int bVideo ,char *pData, int len, void *pParam)
{
    CTestP2PDlg *pDlg = (CTestP2PDlg*)pParam;

    //TRACE("AVDataCallBack... nHandle: %d, bVideo: %d, len: %d\n", nHandle, bVideo, len);
	//DPrintf("\r\n---AVDataCallBack... nHandle: %d, bVideo: %d, len: %d---", nHandle, bVideo, len);

    //video data
    if(bVideo)
    {
        PAV_HEAD phead = (PAV_HEAD)pData;
        //TRACE("video..startcode: 0x%x, type: %d, streamid: %d, len: %d\n", phead->startcode, phead->type, phead->streamid, phead->len);
		//DPrintf("\r\n---video..startcode: 0x%x, type: %d, streamid: %d, len: %d---", phead->startcode, phead->type, phead->streamid, phead->len);

        if (len == sizeof(AV_HEAD) + phead->len)
        {
            //video data
            char *pVideo = pData + sizeof(AV_HEAD);

            if(phead->streamid == 0 || phead->streamid == 1)
            { 
                //BOOL bIFrame = (phead->streamid == 0) ? TRUE : FALSE ;
				BOOL bIFrame = (phead->type == 0) ? TRUE : FALSE ;  //0:I Frame  1:P Frame

                pDlg->m_pVideoPlayer->SendOneFrame(pVideo, phead->len, bIFrame );
            }
        }
    }
    else //audio data
    {
        PAV_HEAD phead = (PAV_HEAD)pData;
        //TRACE("audio..startcode: 0x%x, type: %d, streamid: %d, len: %d\n", phead->startcode, phead->type, phead->streamid, phead->len);
		//DPrintf("\r\n---len=%d, sizeof(AV_HEAD)=%d, phead->len=%d---", len, sizeof(AV_HEAD), phead->len);
        if (len == sizeof(AV_HEAD) + phead->len)
        {
            //char PCMBuf[2048] = {0} ;
            //adpcm data
            //pDlg->m_pAdpcm->ADPCMDecode(pData + sizeof(AV_HEAD), phead->len, PCMBuf);

            //2048大小的PCMBuf数据位PCM数据
			//if (pDlg->m_WaveOut.inlineis_start())
			//{
			//	pDlg->m_WaveOut.input((unsigned char *)PCMBuf, phead->len*4);
			//}

			if(pDlg->m_bAudioPlay)
			{
				EnterCriticalSection(&(pDlg->m_secAudio));
				Frame_Data *p = new Frame_Data;
				memcpy(p, pData, len);
				pDlg->m_FrameQueueAudio.AddTail(p);
				LeaveCriticalSection(&(pDlg->m_secAudio));
			}
        }
    }
}

void CTestP2PDlg::MessageCallBack(long nHandle, int type, char *msg, int len, void *pParam)
{
    CTestP2PDlg *pDlg = (CTestP2PDlg*)pParam;

    TRACE("MessageCallBack...nHandle: %d, type: %d, len: %d\n", nHandle, type, len);
    switch (type)
    {
    case MSG_TYPE_P2P_STATUS:
        {
            int nP2PStatus = *((int*)msg);
            switch (nP2PStatus)
            {
            case P2P_STATUS_CONNECT_TIME_OUT:
                TRACE("P2P_STATUS_CONNECT_TIME_OUT\n");
                break;
            case P2P_STATUS_INVALID_ID:
                TRACE("P2P_STATUS_INVALID_ID\n");
                break;
            case P2P_STATUS_CONNECT_SUCCESS:
                TRACE("P2P_STATUS_CONNECT_SUCCESS\n");
                break;
            case P2P_STATUS_DISCONNECTED:
                TRACE("P2P_STATUS_DISCONNECTED\n");
                break;
            case P2P_STATUS_CONNECT_FAIlED:
                TRACE("P2P_STATUS_CONNECT_FAIlED\n");
                break;
            case P2P_STATUS_CONNECTING:
                TRACE("P2P_STATUS_CONNECTING\n");
                break;
            case P2P_STATUS_DEVICE_NOT_ON_LINE:
                TRACE("P2P_STATUS_DEVICE_NOT_ON_LINE\n");
                break;
            case P2P_STATUS_INVALID_USER_PWD:
                TRACE("P2P_STATUS_INVALID_USER_PWD\n");
                break;
            default:
                break;
            }
        }
        break;
    case MSG_TYPE_P2P_MODE:
        {
            int nP2PMode = *((int*)msg);
            switch (nP2PMode)
            {
            case P2P_MODE_P2P_RELAY:
                TRACE("P2P_MODE_P2P_RELAY\n");
                break;
            case P2P_MODE_P2P_CONNECTED:
                TRACE("P2P_MODE_P2P_CONNECTED\n");
                break;
            default:
                break;
            }
        }
        break;
    case MSG_TYPE_GET_CAMERA_PARAMS:
        {
            if(len != sizeof(STRU_CAMERA_PARAMS))
                break;

            PSTRU_CAMERA_PARAMS pCameraParams = (PSTRU_CAMERA_PARAMS)msg;
            TRACE("resolution: %d, brightness: %d, contrast: %d, hue: %d, saturation: %d, flip: %d\n", pCameraParams->resolution,
                pCameraParams->brightness, pCameraParams->contrast, pCameraParams->hue, pCameraParams->saturation, pCameraParams->flip);

        }
        break;
    case MSG_TYPE_DECODER_CONTROL:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_DECODER_CONTROL result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_SNAPSHOT:
        {
            TRACE("MSG_TYPE_SNAPSHOT len: %d\n", len) ;


			FILE *p;
			p=fopen("c://snapshot.jpg","wb");
			fwrite(msg,1,len,p);

			fclose(p);
			
        }
        break;
    case MSG_TYPE_CAMERA_CONTROL:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_CAMERA_CONTROL result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_SET_NETWORK:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_SET_NETWORK result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_SET_USER:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_SET_USER result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_SET_WIFI:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_SET_WIFI result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_SET_DATETIME:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_SET_DATETIME result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_GET_STATUS:
        {
            TRACE("MSG_TYPE_GET_STATUS\n");
            if (len != sizeof(STRU_CAMERA_STATUS))
                break;
  
            PSTRU_CAMERA_STATUS pCameraStatus = (PSTRU_CAMERA_STATUS)msg;
            TRACE("sysver: %s, devname: %s, devid: %s, alaramstatus: %d, sdcardstatus: %d, sdcardtotalsize: %d, sdcardremainsize: %d, mac: %s, wifimac: %s\n",
                pCameraStatus->sysver, pCameraStatus->devname, pCameraStatus->devid, pCameraStatus->alarmstatus, pCameraStatus->sdcardstatus, pCameraStatus->sdcardtotalsize,
                pCameraStatus->sdcardremainsize, pCameraStatus->mac, pCameraStatus->wifimac);
        }
        break;
    case MSG_TYPE_GET_PTZ_PARAMS:
        {
            TRACE("MSG_TYPE_GET_PTZ_PARAMS\n");
            if (len != sizeof(STRU_PTZ_PARAMS))
                break;
    
            PSTRU_PTZ_PARAMS pPtzParams = (PSTRU_PTZ_PARAMS)msg;
            TRACE("led_mode: %d, ptz_center_onstart: %d, ptz_run_times: %d, ptz_patrol_rate: %d, ptz_patrol_up_rate: %d, ptz_patrol_down_rate: %d, ptz_patrol_left_rate: %d, ptz_patrol_right_rate: %d, disable_preset: %d, ptz_preset: %d",
                pPtzParams->led_mode, pPtzParams->ptz_center_onstart, pPtzParams->ptz_run_times, pPtzParams->ptz_patrol_rate, pPtzParams->ptz_patrol_up_rate, pPtzParams->ptz_patrol_down_rate, pPtzParams->ptz_patrol_left_rate,pPtzParams->ptz_patrol_right_rate, pPtzParams->disable_preset, pPtzParams->ptz_preset);
        }
        break;
    case MSG_TYPE_SET_DDNS:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_SET_DDNS result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_SET_MAIL:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_SET_MAIL result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_SET_FTP:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_SET_FTP result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_SET_ALARM:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_SET_ALARM result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_SET_PTZ:
        {
            int nResult = *((int*)msg);
            TRACE("MSG_TYPE_SET_PTZ result: %d\n", nResult) ;
        }
        break;
    case MSG_TYPE_WIFI_SCAN:
        {
            TRACE("MSG_TYPE_WIFI_SCAN\n");
            if (len != sizeof(STRU_WIFI_SEARCH_RESULT_LIST))
                break;

            PSTRU_WIFI_SEARCH_RESULT_LIST pWifiList = (PSTRU_WIFI_SEARCH_RESULT_LIST)msg;
            int nCount = pWifiList->nResultCount;
            int i;
            for (i = 0; i < nCount; i++)
            {
                TRACE("ssid: %s, mac: %s, security: %d, dbm0: %s, dbm1: %s, mode: %d, channel: %d\n", pWifiList->wifi[i].ssid,
                    pWifiList->wifi[i].mac, pWifiList->wifi[i].security, pWifiList->wifi[i].dbm0, pWifiList->wifi[i].dbm1, pWifiList->wifi[i].mode, pWifiList->wifi[i].channel);
            }

        }
        break;

    case MSG_TYPE_WIFI_PARAMS:
        {
            TRACE("MSG_TYPE_WIFI_PARAMS\n");
            if (len != sizeof(STRU_WIFI_PARAMS))
                break;
            PSTRU_WIFI_PARAMS pWifiParams = (PSTRU_WIFI_PARAMS)msg;
            TRACE("enable: %d, ssid: %s, channel: %d, mode: %d, authtype: %d, encrypt: %d, keyformat: %d, defkey: %d, key1: %s, key2: %s, key3: %s, key4: %s, key1_bits: %d, key2_bits: %d, key3_bits: %d, key4_bits: %d, wpa_psk: %s\n",
                pWifiParams->enable, pWifiParams->ssid, pWifiParams->channel, pWifiParams->mode, pWifiParams->authtype, pWifiParams->encrypt, pWifiParams->keyformat, pWifiParams->defkey,
                pWifiParams->key1, pWifiParams->key2, pWifiParams->key3, pWifiParams->key4, pWifiParams->key1_bits, pWifiParams->key2_bits, pWifiParams->key3_bits, pWifiParams->key4_bits, pWifiParams->wpa_psk);
            
        }
        break;
    case MSG_TYPE_MAIL_PARAMS:
        {
            TRACE("MSG_TYPE_MAIL_PARAMS\n");
            if (len != sizeof(STRU_MAIL_PARAMS))
                break;
         
            PSTRU_MAIL_PARAMS pMailParams = (PSTRU_MAIL_PARAMS)msg;
            TRACE("svr: %s, user: %s, pwd: %s, sender: %s, receiver1: %s, receiver2: %s, receiver3: %s, receiver4: %s, port: %d, ssl: %d\n",
                pMailParams->svr, pMailParams->user, pMailParams->pwd, pMailParams->sender, pMailParams->receiver1, pMailParams->receiver2, pMailParams->receiver3, pMailParams->receiver4, pMailParams->port, pMailParams->ssl);
        }
        break;
    case MSG_TYPE_FTP_PARAMS:
        {
            TRACE("MSG_TYPE_FTP_PARAMS\n");
            if (len != sizeof(STRU_FTP_PARAMS))
                break;

            PSTRU_FTP_PARAMS pFtpParams = (PSTRU_FTP_PARAMS)msg;
            TRACE("svr_ftp: %s, user: %s, dir: %s, port: %d, mode: %d, upload_interval: %d\n", pFtpParams->svr_ftp, pFtpParams->user, pFtpParams->dir, pFtpParams->port, pFtpParams->mode, pFtpParams->upload_interval);

        }
        break;
    case MSG_TYPE_NETWORK_PARAMS:
        {
            TRACE("MSG_TYPE_NETWORK_PARAMS\n");
            if (len != sizeof(STRU_NETWORK_PARAMS))
                break;

            PSTRU_NETWORK_PARAMS pNetworkParams = (PSTRU_NETWORK_PARAMS)msg;
            TRACE("ipaddr: %s, netmask: %s, gateway: %s, dns1: %s, dns2: %s, dhcp: %d, port: %d, rtspport: %d\n", 
                pNetworkParams->ipaddr, pNetworkParams->netmask, pNetworkParams->gateway, pNetworkParams->dns1, pNetworkParams->dns2, pNetworkParams->dhcp, pNetworkParams->port, pNetworkParams->rtspport);
        }
        break;
    case MSG_TYPE_USER_INFO:
        {
            TRACE("MSG_TYPE_USER_INFO\n");
            if (len != sizeof(STRU_USER_INFO))
                break;

            PSTRU_USER_INFO pUserInfo = (PSTRU_USER_INFO)msg;
            TRACE("user1: %s, pwd1: %s, user2: %s, pwd3: %s, user3: %s, pwd3: %s\n", pUserInfo->user1, pUserInfo->pwd1, pUserInfo->user2, pUserInfo->pwd2, pUserInfo->user3, pUserInfo->pwd3);
        }
        break;
    case MSG_TYPE_DDNS_PARAMS:
        {
            TRACE("MSG_TYPE_DDNS_PARAMS\n");
            if (len != sizeof(STRU_DDNS_PARAMS))
                break;
      
            PSTRU_DDNS_PARAMS pDdnsParams = (PSTRU_DDNS_PARAMS)msg;
            TRACE("service: %d, user: %s, pwd: %s, host: %s, proxy_svr: %s, ddns_mode: %d, proxy_port: %d\n",
                pDdnsParams->service, pDdnsParams->user, pDdnsParams->pwd, pDdnsParams->host, pDdnsParams->proxy_svr, pDdnsParams->ddns_mode, pDdnsParams->proxy_port);
        }
        break;
    case MSG_TYPE_DATETIME_PARAMS:
        {
            TRACE("MSG_TYPE_DATETIME_PARAMS\n");
            if (len != sizeof(STRU_DATETIME_PARAMS))
                break;

            PSTRU_DATETIME_PARAMS pDatetimeParams = (PSTRU_DATETIME_PARAMS)msg;
            TRACE("now: %d, tz: %d, ntp_enable: %d, ntp_svr: %s\n", pDatetimeParams->now, pDatetimeParams->tz, pDatetimeParams->ntp_enable, pDatetimeParams->ntp_svr);
        }
        break;
    case MSG_TYPE_ALARM_PARAMS:
        {
            TRACE("MSG_TYPE_ALARM_PARAMS\n");
            if (len != sizeof(STRU_ALARM_PARAMS))
                break;

            PSTRU_ALARM_PARAMS pAlarmParams = (PSTRU_ALARM_PARAMS)msg;
            TRACE("pAlarmParams\n");

        }
        break;
    case MSG_TYPE_ALARM_NOTIFY:
        {
            TRACE("MSG_TYPE_ALARM_NOTIFY  alarmType: %d\n", *((int*)msg));
            
        }
        break;
	case MSG_TYPE_GET_SENSOR_STATUS:
		{
            if (len != sizeof(STRU_SENSOR_STATUS))
                break;

            PSTRU_SENSOR_STATUS sensorStatus = (PSTRU_SENSOR_STATUS)msg;
            TRACE("pAlarmParams\n");
		}
    default:
        break;

    }
}

//4
void CTestP2PDlg::OnBnClickedBtnSetAvdatacallback()
{
    // TODO: 在此添加控件通知处理程序代码
    long nRet = P2PAPI_SetAVDataCallBack(m_hP2PHandle, CTestP2PDlg::AVDataCallBack, this);
	DPrintf("\r\n---P2PAPI_SetAVDataCallBack nRet=%d---", nRet);
}

//3
void CTestP2PDlg::OnBnClickedBtnSetMessageCallback()
{
    // TODO: 在此添加控件通知处理程序代码
    long nRet = P2PAPI_SetMessageCallBack(m_hP2PHandle, CTestP2PDlg::MessageCallBack, this);
	DPrintf("\r\n---P2PAPI_SetMessageCallBack nRet=%d---", nRet);
}

//StartAudio
void CTestP2PDlg::OnBnClickedBtnStartAudio()
{
    // TODO: 在此添加控件通知处理程序代码

	//m_WaveOut.Open();

    //P2PAPI_StartAudio(m_hP2PHandle);

	////////////////////////////////////////
	if (!m_bAudioPlay)
	{
		if (!m_WaveOut.Open()) return ;
		m_bAudioPlay    = TRUE;
		m_bAudioIsFirst = TRUE;

		m_hAPlayThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)APlayWorkThread,this,NULL,NULL); //fixfix 2013-06-23
		BOOL bRet = SetThreadPriority(m_hAPlayThread, THREAD_PRIORITY_NORMAL); //fixfix 2013-06-23

		P2PAPI_StartAudio(m_hP2PHandle);
	}
}

//StopAudio
void CTestP2PDlg::OnBnClickedBtnStopAudio()
{
    // TODO: 在此添加控件通知处理程序代码
    //P2PAPI_StopAudio(m_hP2PHandle);

	//m_WaveOut.Close();

	////////////////////////////////////////

	if (m_bAudioPlay)
	{
		m_bAudioPlay    = FALSE;
		m_bAudioIsFirst = FALSE;

		P2PAPI_StopAudio(m_hP2PHandle);

		if(m_hAPlayThread!=NULL){
			m_bAudioPlay=FALSE;
			WaitForSingleObject(m_hAPlayThread, INFINITE);
			CloseHandle(m_hAPlayThread);
			m_hAPlayThread=NULL;
		}
		m_WaveOut.Close();


		POSITION Pos = m_FrameQueueAudio.GetHeadPosition();
		while(Pos){
			Frame_Data * p = (Frame_Data *)m_FrameQueueAudio.GetNext(Pos);
			delete p;
		}
		m_FrameQueueAudio.RemoveAll();

	}
}

DWORD WINAPI CTestP2PDlg::APlayWorkThread( LPVOID lpParam )
{
	ASSERT(lpParam != NULL);
	CTestP2PDlg *p = (CTestP2PDlg *)lpParam;
	p->APlay();
	TRACE( "APlayWorkThread::APlay()--end---\n");
	return 0;
}


#define MAX_AUDIO_BUF_NUM	2
#define MIN_AUDIO_BUF_NUM	1
void CTestP2PDlg::APlay()
{
	char szbuf[2048]={0};
	Frame_Data *pTemp=NULL;
	AV_HEAD *pAudioHead=NULL;
	int  nAudioFIFONum=0, nAUDIO_BUF_NUM=MAX_AUDIO_BUF_NUM;
	TRACE( "CNetH264StreamPlay::APlay()--begin---\n");

	while(m_bAudioPlay){
		nAudioFIFONum=m_FrameQueueAudio.GetSize();
		//TRACE( "CNetH264StreamPlay::APlay(), nAudioFIFONum=%d\n", nAudioFIFONum);
		if(nAudioFIFONum<nAUDIO_BUF_NUM){
			if(nAUDIO_BUF_NUM==MIN_AUDIO_BUF_NUM) nAUDIO_BUF_NUM=MAX_AUDIO_BUF_NUM;
			Sleep(6);
			continue;
		}else nAUDIO_BUF_NUM=MIN_AUDIO_BUF_NUM;

		EnterCriticalSection(&m_secAudio);
		pTemp=(Frame_Data *)m_FrameQueueAudio.RemoveHead();
		LeaveCriticalSection(&m_secAudio);
		if(pTemp==NULL) continue;

		pAudioHead=(AV_HEAD *)pTemp;
		//DPrintf("\r\n---APlay len=%d, frameno=%d---", pAudioHead->len, pAudioHead->frameno);
		//audio first frame
		if (m_bAudioIsFirst)
		{
			m_bAudioIsFirst = FALSE;
			m_pAdpcm->DecoderClr(pAudioHead->sample, pAudioHead->index);
		}
		m_pAdpcm->ADPCMDecode((char *)((char *)pTemp+sizeof(AV_HEAD)), pAudioHead->len, szbuf);
		m_WaveOut.input((unsigned char*)szbuf, pAudioHead->len*4);

		if (pTemp != NULL)
		{
			delete pTemp;
		}
		pTemp=NULL;
	}
}

//6
void CTestP2PDlg::OnBnClickedBtnStartVideo()
{
    // TODO: 在此添加控件通知处理程序代码
	/////////////////////////////////////
	//P2PAPI_InitialWithServer("EBGBEMBMKGJMGAJPEIGIFKEGHBMCHMJHCKBMBHGFBJNOLCOLCIEBHFOCCHKKJIKPBNMHLHCPPFMFADDFIINOIABFMH"); //VSTC
	//P2PAPI_CreateInstance(&m_hP2PHandle);
	//P2PAPI_Connect(m_hP2PHandle, "VSTC172299NEWGD", "admin", "");
	//P2PAPI_SetAVDataCallBack(m_hP2PHandle, CTestP2PDlg::AVDataCallBack, this);
	///////////////////////////////////
    long nRet = P2PAPI_StartVideo(m_hP2PHandle,0);
	DPrintf("\r\n---P2PAPI_StartVideo nRet=%d---", nRet);
}

void CTestP2PDlg::OnBnClickedBtnStopVideo()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_StopVideo(m_hP2PHandle);
}

void CTestP2PDlg::OnBnClickedBtnCloseP2p()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_Close(m_hP2PHandle);
}

void CTestP2PDlg::OnBnClickedBtnDestroyAllInstance()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_DestroyAllInstance();
}

void CTestP2PDlg::OnBnClickedBtnCloseAll()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_CloseAll();
}

void CTestP2PDlg::OnBnClickedBtnGetCameraParams()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_GET_CAMERA_PARAMS, NULL, 0);
}

void CTestP2PDlg::OnBnClickedBtnDecoderControl()
{
    // TODO: 在此添加控件通知处理程序代码
    int nCmd = CMD_PTZ_UP;
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));
}

void CTestP2PDlg::OnBnClickedBtnSnapshot()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SNAPSHOT, NULL, 0);
}

void CTestP2PDlg::OnBnClickedBtnCameraControl()
{
    // TODO: 在此添加控件通知处理程序代码
    STRU_CAMERA_CONTROL cameracontrol;
    memset(&cameracontrol, 0, sizeof(cameracontrol));
    cameracontrol.param = 1;
    cameracontrol.value = 255;
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_CAMERA_CONTROL, (char*)&cameracontrol, sizeof(cameracontrol));
}

void CTestP2PDlg::OnClose()
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    P2PAPI_CloseAll();
    P2PAPI_DestroyAllInstance();
    P2PAPI_DeInitial();

	if (m_pAdpcm)
	{
		delete m_pAdpcm;
		m_pAdpcm = NULL;
	}

	if (m_pVideoPlayer)
	{
		delete m_pVideoPlayer;
		m_pVideoPlayer = NULL;
	}

    CDialog::OnClose();
}

void CTestP2PDlg::OnBnClickedBtnSetNetwork()
{
    // TODO: 在此添加控件通知处理程序代码
    STRU_NETWORK_PARAMS networkparams;
    memset(&networkparams, 0, sizeof(networkparams));
    strcpy(networkparams.ipaddr, "192.168.1.180");
    strcpy(networkparams.netmask, "255.255.255.0");
    strcpy(networkparams.gateway, "192.168.2.1");
    strcpy(networkparams.dns1, "1.2.3.4");
    strcpy(networkparams.dns2, "5.6.7.8");
    networkparams.port = 898;
    networkparams.rtspport = 0;
    networkparams.dhcp = 0;
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SET_NETWORK, (char*)&networkparams, sizeof(networkparams));

}

void CTestP2PDlg::OnBnClickedBtnReboot()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_REBOOT_DEVICE, NULL, 0);
}

void CTestP2PDlg::OnBnClickedBtnRestore()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_RESTORE_FACTORY, NULL, 0);
}

void CTestP2PDlg::OnBnClickedBtnWifiScan()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_WIFI_SCAN, NULL, 0);
}

void CTestP2PDlg::OnBnClickedBtnGetParams()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_GET_PARAMS, NULL, 0);
}

void CTestP2PDlg::OnBnClickedBtnGetStatus()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_GET_STATUS, NULL, 0);
}

void CTestP2PDlg::OnBnClickedBtnGetPtzParams()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_GET_PTZ_PARAMS, NULL, 0);
}

void CTestP2PDlg::OnBnClickedBtnGetAlarmLog()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_GET_ALARM_LOG, NULL, 0);
}

void CTestP2PDlg::OnBnClickedBtnStartTalk()
{
    // TODO: 在此添加控件通知处理程序代码
    //P2PAPI_StartTalk(m_hP2PHandle);
    //if (m_hTalkThread == NULL)
    //{
    //   m_bTalkThreadRuning = TRUE;
    //    m_hTalkThread = CreateThread(NULL, 0, TalkThread, this, 0, NULL);
    //}
	///////////////////////////////////
	if (!m_bAudioTalk)
	{
		long nRet = P2PAPI_StartTalk(m_hP2PHandle);
		DPrintf("\r\n---P2PAPI_StartTalk nRet=%d---", nRet);

		m_WaveIn.Ini(TalkDataCallBack, (void*)this);
		BOOL bRet = m_WaveIn.OpenRecord();
		DPrintf("\r\n---P2PAPI_StartTalk bRet=%d---", bRet);

		m_bAudioTalk = TRUE;
	}
}

void CTestP2PDlg::OnBnClickedBtnStopTalk()
{
    // TODO: 在此添加控件通知处理程序代码
    /*
	if (m_hTalkThread != NULL)
    {
        m_bTalkThreadRuning = FALSE;
        DWORD dwWait = WaitForSingleObject(m_hTalkThread, INFINITE);
        if (dwWait == WAIT_OBJECT_0)
        {
            CloseHandle(m_hTalkThread);
            m_hTalkThread = NULL;
        }
    }
    P2PAPI_StopTalk(m_hP2PHandle) ;
	*/
	///////////////////////////////////
	if (m_bAudioTalk)
	{
		m_WaveIn.CloseRecord();
		m_WaveIn.Release();

		P2PAPI_StopTalk(m_hP2PHandle) ;

		m_bAudioTalk = FALSE;
	}
}

BOOL  CALLBACK CTestP2PDlg::TalkDataCallBack(const char *pData , unsigned int nDataLen, void *pContext)
{
	CTestP2PDlg *pThisDlg = (CTestP2PDlg *)pContext;
	if (NULL != pThisDlg)
	{
		//发送对讲数据
		P2PAPI_TalkData(pThisDlg->m_hP2PHandle, (char *)pData, nDataLen);
	}
	return TRUE;
}

DWORD CTestP2PDlg::TalkThread(LPVOID lPARAM)
{
    CTestP2PDlg *pdlg = (CTestP2PDlg*)lPARAM;
    pdlg->TalkProcess();
    return 0;
}

void CTestP2PDlg::TalkProcess()
{
    while(m_bTalkThreadRuning)
    {
        Sleep(100);
        /*
        char PCMBuf[1024];
        char pAdpcm[256];

        //使用1024大小的PCM，压缩成256大小的ADPCM
        m_pAdpcm->ADPCMEncode(PCMBuf, 1024, pAdpcm);

        //发送对讲数据
        P2PAPI_TalkData(m_hP2PHandle, pAdpcm, 256); //长度必须是256长度的ADPCM数据，其他的长度不合法

        */
    }
}
void CTestP2PDlg::OnBnClickedBtnDecoderControlLeft()
{
    // TODO: 在此添加控件通知处理程序代码
    int nCmd = CMD_PTZ_LEFT;
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));
}

void CTestP2PDlg::OnBnClickedBtnDecoderControlRight()
{
    // TODO: 在此添加控件通知处理程序代码
    int nCmd = CMD_PTZ_RIGHT;
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));
}

void CTestP2PDlg::OnBnClickedBtnDecoderControlDown()
{
    // TODO: 在此添加控件通知处理程序代码
    int nCmd = CMD_PTZ_DOWN;
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));
}

void CTestP2PDlg::OnBnClickedBtnSetDate()
{
    // TODO: 在此添加控件通知处理程序代码
    STRU_DATETIME_PARAMS DatetimeParams;
    memset(&DatetimeParams, 0, sizeof(DatetimeParams));
    strcpy(DatetimeParams.ntp_svr, "time.nist.gov");
    DatetimeParams.ntp_enable = 1;
    DatetimeParams.tz = 0;//utc//-28800; //北京
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SET_DATETIME, (char*)&DatetimeParams, sizeof(DatetimeParams));

}

void CTestP2PDlg::OnBnClickedBtnSetUser()
{
    // TODO: 在此添加控件通知处理程序代码
    STRU_USER_INFO UserInfo;
    memset(&UserInfo, 0, sizeof(UserInfo));
    strcpy(UserInfo.user1, "aaaaa");
    strcpy(UserInfo.pwd1, "aaaaa123");
    strcpy(UserInfo.user2, "bbbbb");
    strcpy(UserInfo.pwd2, "bbbbb123");
    strcpy(UserInfo.user3, "ccccc");
    strcpy(UserInfo.pwd3, "ccccc123");

    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SET_USER, (char*)&UserInfo, sizeof(UserInfo));
}

void CTestP2PDlg::OnBnClickedBtnSetMail()
{
    // TODO: 在此添加控件通知处理程序代码
    STRU_MAIL_PARAMS MailParams;
    memset(&MailParams, 0, sizeof(MailParams));
    strcpy(MailParams.svr, "smtp.163.com");
    strcpy(MailParams.user, "test");
    strcpy(MailParams.pwd, "test123");
    strcpy(MailParams.sender, "test@163.com");
    strcpy(MailParams.receiver1, "123445@qq.com");
    strcpy(MailParams.receiver2, "222333@163.com");
    strcpy(MailParams.receiver3, "asdfs@163.com");
    strcpy(MailParams.receiver4, "bbbbdd@163.com");
    MailParams.port = 100;
    MailParams.ssl = 1; //0:none 1:SSL 2:TLS

    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SET_MAIL, (char*)&MailParams, sizeof(MailParams));
}

void CTestP2PDlg::OnBnClickedBtnSetFtp()
{
    // TODO: 在此添加控件通知处理程序代码
    STRU_FTP_PARAMS FtpParams;
    memset(&FtpParams, 0, sizeof(FtpParams));
    strcpy(FtpParams.svr_ftp, "192.168.1.121");
    strcpy(FtpParams.user, "testuser");
    strcpy(FtpParams.pwd, "123456");
    strcpy(FtpParams.dir, "/");
    FtpParams.port = 2121;
    FtpParams.mode = 0;
    FtpParams.upload_interval = 10;

    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SET_FTP, (char*)&FtpParams, sizeof(FtpParams));

}

void CTestP2PDlg::OnBnClickedBtnSetWifi()
{
    // TODO: 在此添加控件通知处理程序代码
    STRU_WIFI_PARAMS WifiParams;
    memset(&WifiParams, 0, sizeof(WifiParams));

    WifiParams.enable = 1;
    strcpy(WifiParams.ssid, "testwifi");
    WifiParams.channel = 0;
    WifiParams.mode = 2;
    WifiParams.authtype = 2;
    //WifiParams.encrypt = 1;
    //WifiParams.keyformat = 1;
    //WifiParams.defkey = 1;
    //strcpy(WifiParams.key1, "12345");
    //WifiParams.key1_bits = 1;
    //strcpy(WifiParams.key2, "aaaaa");
    //WifiParams.key2_bits = 1;
    //strcpy(WifiParams.key3, "bbbbb");
    //WifiParams.key3_bits = 1;
    //strcpy(WifiParams.key4, "ccccc");
    //WifiParams.key4_bits = 1;
    strcpy(WifiParams.wpa_psk, "aaaaaaaaaaa");

    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SET_WIFI, (char*)&WifiParams, sizeof(WifiParams));
}

void CTestP2PDlg::OnBnClickedBtnSetAlarm()
{
    // TODO: 在此添加控件通知处理程序代码
    STRU_ALARM_PARAMS AlarmParams;
    memset(&AlarmParams, 0, sizeof(AlarmParams));

    AlarmParams.motion_armed = 1;
    AlarmParams.motion_sensitivity = 4;
    AlarmParams.input_armed = 1;
    AlarmParams.ioin_level = 1;
    AlarmParams.iolinkage = 1;
    AlarmParams.alarmpresetsit = 3;
    AlarmParams.mail = 1;
    AlarmParams.snapshot = 1;
    AlarmParams.record = 1;
    AlarmParams.upload_interval = 15;
    AlarmParams.schedule_enable = 1;
    AlarmParams.schedule_mon_0 = 0xFFFFFFFF;
    AlarmParams.schedule_mon_1 = 0xFFFFFFFF;
    AlarmParams.schedule_mon_2 = 0xFFFFFFFF;
    AlarmParams.schedule_thu_0 = 0xFFFFFFFF;
    AlarmParams.schedule_thu_1 = 0xFFFFFFFF;
    AlarmParams.schedule_thu_2 = 0xFFFFFFFF;
    AlarmParams.schedule_wed_0 = 0xFFFFFFFF;
    AlarmParams.schedule_wed_1 = 0xFFFFFFFF;
    AlarmParams.schedule_wed_2 = 0xFFFFFFFF;
    AlarmParams.schedule_thu_0 = 0xFFFFFFFF;
    AlarmParams.schedule_thu_1 = 0xFFFFFFFF;
    AlarmParams.schedule_thu_2 = 0xFFFFFFFF;
    AlarmParams.schedule_fri_0 = 0xFFFFFFFF;
    AlarmParams.schedule_fri_1 = 0xFFFFFFFF;
    AlarmParams.schedule_fri_2 = 0xFFFFFFFF;
    AlarmParams.schedule_sat_0 = 0xFFFFFFFF;
    AlarmParams.schedule_sat_1 = 0xFFFFFFFF;
    AlarmParams.schedule_sat_2 = 0xFFFFFFFF;
    AlarmParams.schedule_sun_0 = 0xFFFFFFFF;
    AlarmParams.schedule_sun_1 = 0xFFFFFFFF;
    AlarmParams.schedule_sun_2 = 0xFFFFFFFF;

    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SET_ALARM, (char*)&AlarmParams, sizeof(AlarmParams));
}

void CTestP2PDlg::OnBnClickedBtnSetDeviceName()
{
    // TODO: 在此添加控件通知处理程序代码
    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SET_DEVNAME, (char*)"aaaa", strlen("aaaa")) ;
}

void CTestP2PDlg::OnBnClickedBtnSetPtz()
{
    // TODO: 在此添加控件通知处理程序代码
    STRU_PTZ_PARAMS ptzParams;
    memset(&ptzParams, 0, sizeof(ptzParams));

    ptzParams.disable_preset = 1;
    ptzParams.led_mode = 1;
    ptzParams.ptz_center_onstart = 1;
    ptzParams.ptz_patrol_rate = 5;
    ptzParams.ptz_patrol_up_rate = 5;
    ptzParams.ptz_patrol_down_rate = 5;
    ptzParams.ptz_patrol_left_rate = 5;
    ptzParams.ptz_patrol_right_rate = 5;
    ptzParams.ptz_run_times = 2;
    ptzParams.ptz_preset = 3;

    P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_SET_PTZ, (char*)&ptzParams, sizeof(ptzParams));
}

void CTestP2PDlg::OnBnClickedBtnSetpreset()//设置预置位
{
	// TODO: 在此添加控件通知处理程序代码
	CMenu menu;
	menu.LoadMenu(IDR_MENU_SETPRESET);
	RECT rect;
	GetDlgItem(IDC_BTN_SETPRESET)->GetWindowRect(&rect);
	CPoint pos(rect.left+2, rect.bottom+1);  
	CMenu* pMenu = menu.GetSubMenu(0); 
	if (NULL != pMenu)
	{
		SetForegroundWindow();
		pMenu->TrackPopupMenu (TPM_RIGHTBUTTON, pos.x, pos.y, this); 
		menu.DestroyMenu();
	}
}

void CTestP2PDlg::OnBnClickedBtnGetpreset()//转动到预置位
{
	// TODO: 在此添加控件通知处理程序代码
	CMenu menu;
	menu.LoadMenu(IDR_MENU_GETPRESET);
	RECT rect;
	GetDlgItem(IDC_BTN_GETPRESET)->GetWindowRect(&rect);
	CPoint pos(rect.left+2, rect.bottom+1); 
	CMenu* pMenu = menu.GetSubMenu(0); 
	if (NULL != pMenu)
	{
		SetForegroundWindow();
		pMenu->TrackPopupMenu (TPM_RIGHTBUTTON, pos.x, pos.y, this); 
		menu.DestroyMenu();
	}
}

BOOL CTestP2PDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	int nCmd = -1;
	switch (wParam)
	{
	case ID_SETPRESET1:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET0 ;
			
			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET2:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET1 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}

	case ID_SETPRESET3:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET2 ;
			
			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}

	case ID_SETPRESET4:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET3 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET5:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET4 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}

	case ID_SETPRESET6:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET5 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}

	case ID_SETPRESET7:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET6 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET8:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET7 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET9:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET8 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET10:
		{
			nCmd =  CMD_PTZ_PREFAB_BIT_SET9 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET11:
		{
			int nCmd =  CMD_PTZ_PREFAB_BIT_SETA ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET12:
		{
			int nCmd =  CMD_PTZ_PREFAB_BIT_SETB ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET13:
		{
			int nCmd =  CMD_PTZ_PREFAB_BIT_SETC ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET14:
		{
			int nCmd =  CMD_PTZ_PREFAB_BIT_SETD ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET15:
		{
			int nCmd =  CMD_PTZ_PREFAB_BIT_SETE ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_SETPRESET16:
		{
			int nCmd =  CMD_PTZ_PREFAB_BIT_SETF ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}

	case ID_GETPRESET1:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN0 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}

	case ID_GETPRESET2:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN1 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;

		}
	case ID_GETPRESET3:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN2 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_GETPRESET4:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN3 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;

		}
	case ID_GETPRESET5:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN4 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;

		}
	case ID_GETPRESET6:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN5 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_GETPRESET7:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN6 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_GETPRESET8:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN7 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_GETPRESET9:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN8 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_GETPRESET10:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUN9 ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_GETPRESET11:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUNA ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_GETPRESET12:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUNB ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_GETPRESET13:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUNC ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}
	case ID_GETPRESET14:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUND ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}

	case ID_GETPRESET15:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUNE ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}

	case ID_GETPRESET16:
		{
			int nCmd = CMD_PTZ_PREFAB_BIT_RUNF ;

			P2PAPI_SendMessage(m_hP2PHandle, MSG_TYPE_DECODER_CONTROL, (char*)&nCmd, sizeof(int));

			break;
		}


	default:
		break;
	}

	return CDialog::OnCommand(wParam, lParam);
}

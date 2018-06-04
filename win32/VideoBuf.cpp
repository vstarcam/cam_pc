#include "StdAfx.h"
#include "VideoBuf.h"

CVideoBuf::CVideoBuf(void)
{
	m_bWaitForIFrame = TRUE;
}

CVideoBuf::~CVideoBuf(void)
{
	FreeAllFrame();	
}

void CVideoBuf::SendOneFrame(char* pBuf, int Len, BOOL bIFrame)
{
	VIDEO_BUF_LIST VideoBuf;
	list<VIDEO_BUF_LIST>::iterator iter;

	m_cs.Lock();

//	TRACE("VideoBuf size: %d\n", m_VideoBufList.size());

// 	//----- ´¦Àí¶ªÖ¡ ------------------------------
// 	if (m_VideoBufList.size() > 10 && bIFrame)
// 	{
// 		TRACE("throw the frame...\n");
// 		for (iter=m_VideoBufList.begin(); iter!=m_VideoBufList.end(); ++iter)
// 		{
// 			char *p = (*iter).pBuf;
// 			//TRACE("FreeAllFrame: p: 0x%x\n", p);
// 			SAFE_DELETE(p);
// 		}
// 
// 		m_VideoBufList.clear();
// 
// 		m_bWaitForIFrame = TRUE;
// 	}
// 
// 	if (m_bWaitForIFrame && bIFrame)
// 	{
// 		m_bWaitForIFrame = FALSE;
// 	}
// 
// 	if (m_bWaitForIFrame)
// 	{
// 		SAFE_DELETE(pBuf);
// 		m_cs.Unlock();
// 		return;
// 	}

	memset(&VideoBuf, 0, sizeof(VIDEO_BUF_LIST));

    char *pVideoBuf = new char[Len] ;
    memcpy(pVideoBuf, pBuf, Len) ;
	VideoBuf.pBuf = pVideoBuf;
	VideoBuf.len = Len;
	VideoBuf.bIFrame = bIFrame;
	m_VideoBufList.push_back(VideoBuf);	

	m_cs.Unlock();

}

BOOL CVideoBuf::GetOneFrame(char *&pBuf, int& Len, BOOL& bIFrame)
{
	VIDEO_BUF_LIST VideoBuf;

	m_cs.Lock();
	if (m_VideoBufList.empty())
	{
		m_cs.Unlock();
		return FALSE;
	}	

	memset(&VideoBuf, 0, sizeof(VIDEO_BUF_LIST));
	VideoBuf = m_VideoBufList.front();
	pBuf = VideoBuf.pBuf;
	Len = VideoBuf.len;	
	bIFrame = VideoBuf.bIFrame;

	m_VideoBufList.pop_front();

	m_cs.Unlock();

	return TRUE;
}

void CVideoBuf::FreeAllFrame()
{
	list<VIDEO_BUF_LIST>::iterator iter;

	//TRACE("Call CVideoBuf::FreeAllFrame()\n");

	m_cs.Lock();
	for (iter=m_VideoBufList.begin(); iter!=m_VideoBufList.end(); ++iter)
	{
		char *p = (*iter).pBuf;
		//TRACE("FreeAllFrame: p: 0x%x\n", p);
		SAFE_DELETE(p);
	}

	m_VideoBufList.clear();

	m_bWaitForIFrame = TRUE;

	m_cs.Unlock();
}
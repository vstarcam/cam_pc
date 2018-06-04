#pragma once

#ifndef _VIDEO_BUF_H_
#define _VIDEO_BUF_H_

typedef struct tagVIDEO_BUF_LIST
{
	int len;
	char  *pBuf;
	BOOL bIFrame;
}VIDEO_BUF_LIST,*PVIDEO_BUF_LIST;

class CVideoBuf
{
public:
	CVideoBuf(void);
	~CVideoBuf(void);

	void SendOneFrame(char* pBuf, int Len, BOOL bIFrame);
	BOOL GetOneFrame(char *&pBuf, int &Len, BOOL& bIFrame);
	void FreeAllFrame();

private:
	list<VIDEO_BUF_LIST> m_VideoBufList;
	CCriticalSection m_cs;

	BOOL m_bWaitForIFrame;
};

#endif

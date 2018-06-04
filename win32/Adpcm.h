#pragma once
class CAdpcm
{
public:
	CAdpcm(void);
	~CAdpcm(void);

	int g_nEnAudioPreSample;
	int g_nEnAudioIndex;
	int g_nDeAudioPreSample;
	int g_nDeAudioIndex;

	void EncoderClr(void);
	void DecoderClr(int nSample, int nIndex);

	void ADPCMEncode(unsigned char *pRaw, int nLenRaw, unsigned char *pBufEncoded);
	void ADPCMDecode(char *pDataCompressed, int nLenData, char *pDecoded);
};


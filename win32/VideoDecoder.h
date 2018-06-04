#pragma once

class CVideoDecoder
{
public:
    CVideoDecoder(void);
    ~CVideoDecoder(void);

    int DecoderFrame(char *pbuf, int len, char *outbuf, int *width, int *height);

private:
    AVFormatContext		*m_pFormatCtx;
    AVCodecContext		*m_pCodecCtx;
    AVFrame				*m_pFrame;
    AVCodec		*m_pCodec;
};

#include "StdAfx.h"
#include "VideoDecoder.h"

CVideoDecoder::CVideoDecoder(void)
{
    m_pCodecCtx = NULL;
    m_pFormatCtx = NULL;
    m_pFrame = NULL;
    m_pCodec = NULL;

    av_register_all();

    m_pCodec = avcodec_find_decoder(CODEC_ID_H264);
    if (!m_pCodec)
    {
        return ;
    }

    m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
    if (!m_pCodecCtx)
    {
        return ;
    }

    if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL) >= 0)
    {
        m_pFrame  = avcodec_alloc_frame();     
    } 
   

}

CVideoDecoder::~CVideoDecoder(void)
{
    if(m_pFrame)
    {
        av_free(m_pFrame);
        m_pFrame = NULL;
    }

    if(m_pCodecCtx)	
    {       
        avcodec_close(m_pCodecCtx);       
        m_pCodecCtx = NULL;   
    }
}

int CVideoDecoder::DecoderFrame(char *pbuf, int len, char *YUVBuf, int *width, int *height)
{
    AVPacket	avpkt;
    av_init_packet(&avpkt);
    avpkt.data = (uint8_t *)pbuf;
    avpkt.size = len;

    int consumed_bytes;
    avcodec_decode_video2(m_pCodecCtx, m_pFrame, &consumed_bytes, &avpkt);
    if (consumed_bytes > 0)
    {
        int nWidth = m_pCodecCtx->width;
        int nHeight = m_pCodecCtx->height;

        *width = nWidth;
        *height = nHeight;

        char *p = YUVBuf;
        int i, j, k;
        for(i=0;i<nHeight;i++)
            memcpy(p + i*nWidth, m_pFrame->data[0]+m_pFrame->linesize[0]*i, nWidth);
        
        p += nWidth * nHeight;

        for(j=0;j<nHeight/2;j++)
            memcpy(p + j * nWidth / 2, m_pFrame->data[1]+m_pFrame->linesize[1]*j, nWidth/2);

        p += nWidth * nHeight / 4;

        for(k=0;k<nHeight/2;k++)
            memcpy(p + k * nWidth / 2, m_pFrame->data[2]+m_pFrame->linesize[2]*k, nWidth/2);

    }

    return consumed_bytes;
}

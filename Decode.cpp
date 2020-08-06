#include "Decode.h"
#include <iostream>
using namespace std;

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
	//#include "SDL.h"
};


Decode::Decode()
{
	mpCodecCtx = NULL;
}


Decode::~Decode()
{
}

/**
* 打开解码器上下文
*
* @param 解码器上下文
*/
void Decode::openCodecParam(AVCodecContext *pCodecCtx)
{
	m_mutex.lock();
	if (!pCodecCtx)
	{
		m_mutex.unlock();
		return;
	}
	mpCodecCtx = pCodecCtx;
	AVCodec *pCodec = avcodec_find_decoder(mpCodecCtx->codec_id);
	if (!pCodec)
	{
		m_mutex.unlock();
		avcodec_free_context(&mpCodecCtx);
		cout << "pCodec is NULL!!!!" << endl;
		return;
	}

	int nRet = avcodec_open2(mpCodecCtx, pCodec, NULL);
	if (nRet != 0)
	{
		m_mutex.unlock();
		avcodec_free_context(&mpCodecCtx);
		cout << "avcodec_open2 error!!!" << endl;
		return;
	}

	m_mutex.unlock();
}

/**
* 发送一帧数据去解码
*
* @param pkt数据帧
* @return 发送成功与否
*/
bool Decode::Send(AVPacket *pkt)
{
	m_mutex.lock();
	if (!mpCodecCtx || !pkt || !pkt->data || !pkt->size)
	{
		m_mutex.unlock();
		return false;
	}
	int nRet = avcodec_send_packet(mpCodecCtx, pkt);
	if (nRet != 0)
	{
		m_mutex.unlock();
		av_packet_free(&pkt);
		cout << "avcodec_send_packet error!!!!" << endl;
		return false;
	}
	m_mutex.unlock();
	av_packet_free(&pkt);
	return true;
}

/**
* 接收一帧解完码的数据
*
* @return 接收数据帧frame
*/
AVFrame *Decode::Recv()
{
	m_mutex.lock();
	if (!mpCodecCtx)
	{
		m_mutex.unlock();
		return NULL;
	}
	AVFrame *pFrame = av_frame_alloc();
	int nRet = avcodec_receive_frame(mpCodecCtx, pFrame);
	if (nRet != 0)
	{
		m_mutex.unlock();
		av_frame_free(&pFrame);
		cout << "avcodec_receive_frame error!!!" << endl;
		return NULL;
	}
	m_mutex.unlock();
	return pFrame;
}
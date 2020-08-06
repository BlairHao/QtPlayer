#include "XDemux.h"
#include <iostream>
using namespace std;
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}


XDemux::XDemux(QObject *parent)
	:QObject(parent)
{
	av_register_all();
	avformat_network_init();
	mpFormatCtx = NULL;
	mnSrcWidth = 0;
	mnSrcHeight = 0;
	mnVideoIndex = 0;
	mnAudioIndex = 0;
	mbRepeatPlay = true;
	connect(this, SIGNAL(signal_read_finish()), this, SLOT(test()));
}


XDemux::~XDemux()
{
}

void XDemux::test()
{
	cout << "ahsgdhjashdk" << endl;
}

void XDemux::openMediaFile(const char *strFilePath)
{
	mbRepeatPlay = true;
	/***********  解封装  ************/
	AVDictionary *p_option = NULL;

	//设置rtsp流的传输方式为tcp
	av_dict_set(&p_option, "rtsp_transport", "tcp", 0);
	//设置最大的网络延时时间为500毫秒
	av_dict_set(&p_option, "max_delay", "500", 0);

	mpFormatCtx = avformat_alloc_context();
	int nRet = avformat_open_input(&mpFormatCtx, strFilePath, 0, NULL);
	if (nRet != 0)
	{
		avformat_free_context(mpFormatCtx);
		cout << "avformat_open_input error!!!" << endl;
		return;
	}
	nRet = avformat_find_stream_info(mpFormatCtx, NULL);
	if (nRet < 0)
	{
		cout << "avformat_find_stream_info error!!!" << endl;
		return;
	}
	cout << "extensions: " << mpFormatCtx->iformat->extensions << endl;
	cout << "long_name: " << mpFormatCtx->iformat->long_name<< endl;
	cout << "name: " << mpFormatCtx->iformat->name << endl;

	mnVideoIndex = av_find_best_stream(mpFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, -1);
	mnAudioIndex = av_find_best_stream(mpFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, -1);
	mnSrcWidth = mpFormatCtx->streams[mnVideoIndex]->codec->width;
	mnSrcHeight = mpFormatCtx->streams[mnVideoIndex]->codec->height;
	cout << "mnWidth: " << mnSrcWidth << endl;
	cout << "mnHeight: " << mnSrcHeight << endl;
}

AVCodecContext *XDemux::copyVideoParam()
{
	m_mutex.lock();
	if (!mpFormatCtx)
	{
		m_mutex.unlock();
		return NULL;
	}
	AVCodec *pCodec = avcodec_find_decoder(mpFormatCtx->streams[mnVideoIndex]->codec->codec_id);
	if (!pCodec)
	{
		m_mutex.unlock();
		return NULL;
	}
	AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx)
	{
		m_mutex.unlock();
		avcodec_free_context(&pCodecCtx);
		return NULL;
	}
	int nRet = avcodec_copy_context(pCodecCtx, mpFormatCtx->streams[mnVideoIndex]->codec);
	if (nRet != 0)
	{
		m_mutex.unlock();
		avcodec_free_context(&pCodecCtx);
		return NULL;
	}
	m_mutex.unlock();
	return pCodecCtx;
}

AVPacket *XDemux::readOnePacket()
{
	m_mutex.lock();
	if (!mpFormatCtx)
	{
		m_mutex.unlock();
		return NULL;
	}
	AVPacket *pkt = av_packet_alloc();
	int nRet = av_read_frame(mpFormatCtx, pkt);
	if (nRet < 0)
	{
		m_mutex.unlock();
		if (mbRepeatPlay)
		{
			emit signal_read_finish();
			cout << "av_read_frame finish!!!!!!!!!!!!!!!" << endl;
			mbRepeatPlay = false;
		}
		av_packet_free(&pkt);
		return NULL;
	}
	m_mutex.unlock();
	return pkt;

}

/**
* 判断是否是视频帧
*
* @param true: Video  false: Audio
*/
bool XDemux::IsVideo(AVPacket *pkt)
{
	if (!pkt) return false;
	if (pkt->stream_index == AVMEDIA_TYPE_VIDEO)
	{
		return true;
	}
	return false;
}


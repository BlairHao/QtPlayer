#include "XDemux.h"
#include <iostream>
using namespace std;
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
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
	mnSampleRate = 0;
	mnChannels = 0;
	mlTotalMs = 0;
	mbRepeatPlay = true;
	connect(this, SIGNAL(signal_read_finish()), this, SLOT(test()));
}


XDemux::~XDemux()
{
	close();
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

	//mpFormatCtx = avformat_alloc_context();
	mpFormatCtx = NULL;
	int nRet = avformat_open_input(&mpFormatCtx, strFilePath, 0, &p_option);
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
	//cout << "extensions: " << mpFormatCtx->iformat->extensions << endl;
	//cout << "long_name: " << mpFormatCtx->iformat->long_name<< endl;
	//cout << "name: " << mpFormatCtx->iformat->name << endl;

	//总时长(毫秒)
	mlTotalMs = mpFormatCtx->duration / (AV_TIME_BASE / 1000);
	cout << "mlTotalMs: " << mlTotalMs << endl;

	//视频流信息
	mnVideoIndex = av_find_best_stream(mpFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, -1);
	mnAudioIndex = av_find_best_stream(mpFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, -1);
	mnSrcWidth = mpFormatCtx->streams[mnVideoIndex]->codec->width;
	mnSrcHeight = mpFormatCtx->streams[mnVideoIndex]->codec->height;

	cout << "mnVideoIndex: " << mnVideoIndex << endl;
	cout << "mnAudioIndex: " << mnAudioIndex << endl;

	cout << "mnWidth: " << mnSrcWidth << endl;
	cout << "mnHeight: " << mnSrcHeight << endl;

	//音频流信息
	mnSampleRate = mpFormatCtx->streams[mnAudioIndex]->codec->sample_rate;
	mnChannels = mpFormatCtx->streams[mnAudioIndex]->codec->channels;
	cout << "mnSampleRate: " << mnSampleRate << endl;
	cout << "mnChannels: " << mnChannels << endl;
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

AVCodecContext *XDemux::copyAudioParam()
{
	m_mutex.lock();
	if (!mpFormatCtx)
	{
		m_mutex.unlock();
		return NULL;
	}
	AVCodec *pCodec = avcodec_find_decoder(mpFormatCtx->streams[mnAudioIndex]->codec->codec_id);
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
	int nRet = avcodec_copy_context(pCodecCtx, mpFormatCtx->streams[mnAudioIndex]->codec);
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
	//pts转换为毫秒
	pkt->pts = pkt->pts*(1000 * (r2d(mpFormatCtx->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts*(1000 * (r2d(mpFormatCtx->streams[pkt->stream_index]->time_base)));
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

//清空读取缓冲
void XDemux::clear()
{
	m_mutex.lock();
	if (!mpFormatCtx)
	{
		m_mutex.unlock();
		return;
	}
	//清理读取缓冲
	avformat_flush(mpFormatCtx);
	m_mutex.unlock();
}

void XDemux::close()
{
	m_mutex.lock();
	if (!mpFormatCtx)
	{
		m_mutex.unlock();
		return;
	}
	avformat_close_input(&mpFormatCtx);
	mlTotalMs = 0;
	m_mutex.unlock();
}

//seek 位置 position 0.0~1.0
bool XDemux::seek(double nPosition)
{
	m_mutex.lock();
	if (!mpFormatCtx)
	{
		m_mutex.unlock();
		return false;
	}
	//清理读取缓冲
	avformat_flush(mpFormatCtx);

	long long seekPos = 0;
	seekPos = mpFormatCtx->streams[mnVideoIndex]->duration * nPosition;
	cout << "XDemux nPosition: " << nPosition<<"  seekPos: "<< seekPos;
	int nRet = av_seek_frame(mpFormatCtx, mnVideoIndex, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	m_mutex.unlock();
	if (nRet < 0)
	{
		return false;
	}
	return true;
}

AVPacket *XDemux::readVideo()
{
	m_mutex.lock();
	if (!mpFormatCtx)
	{
		m_mutex.unlock();
		return NULL;
	}
	m_mutex.unlock();
	AVPacket *pkt = NULL;
	for (int i=0; i<20; i++)
	{
		pkt = readOnePacket();
		if (!pkt)
		{
			break;
		}
		if (pkt->stream_index == mnVideoIndex)
		{
			break;
		}
		av_packet_free(&pkt);
	}
	//cout << "pkt->pts: " << pkt->pts*(1000 * (r2d(mpFormatCtx->streams[pkt->stream_index]->time_base)));
	return pkt;
}



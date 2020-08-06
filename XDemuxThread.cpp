#include "XDemuxThread.h"
#include <iostream>
using namespace std;
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

XDemuxThread::XDemuxThread()
{
	av_register_all();
	avformat_network_init();
	mpXDemux = NULL;
	mpVideoCall = NULL;
	mpVideoThread = NULL;
	mpXDemux = new XDemux;
	//connect(mpXDemux, SIGNAL(signal_read_finish()), this, SLOT(repeatPlay()));
}


XDemuxThread::~XDemuxThread()
{
	if (mpVideoThread)
	{
		mpVideoThread->mbIsExit = true;
		mpVideoThread->wait();
		delete mpVideoThread;
		mpVideoThread = NULL;
	}
	mbIsExit = true;
	wait();
}

void XDemuxThread::run()
{
	while (!mbIsExit)
	{
		mMutex.lock();
		if (!mpXDemux)
		{
			mMutex.unlock();
			msleep(5);
			continue;
		}
		if (mbIsPause)
		{
			mMutex.unlock();
			continue;
		}
		if (!mpXDemux->mbRepeatPlay)
		{
			repeatPlay();
		}
		//音视频同步
		/*if (pVideoThread && m_pXAudioThread)
		{
		pts = m_pXAudioThread->pts;
		m_pXVideoThread->synpts = m_pXAudioThread->pts;
		}*/

		AVPacket *pkt = mpXDemux->readOnePacket();
		if (!pkt)
		{
			mMutex.unlock();
			msleep(5);
			continue;
		}
		if (mpXDemux->IsVideo(pkt))
		{
			if (mpVideoThread) mpVideoThread->Push(pkt);
		}
		else
		{
			//if (m_pXAudioThread) m_pXAudioThread->Push(pkt);
		}

		mMutex.unlock();
		msleep(1);
	}
}

void XDemuxThread::openMediaFile(const char *strFilePath, IVideoCall *pCall)
{
	mMutex.lock();
	if (!mpXDemux)
	{
		mpXDemux = new XDemux;
	}
	if (!mpVideoThread)
	{
		mpVideoThread = new XVideoThread;
	}
	mpVideoCall = pCall;
	m_strFilePath = strFilePath;
	mpXDemux->openMediaFile(strFilePath);
	mpVideoThread->open(mpXDemux->copyVideoParam(), mpVideoCall, mpXDemux->mnSrcWidth, mpXDemux->mnSrcHeight);
	mMutex.unlock();
}

void XDemuxThread::openMediaFile(const char *strFilePath)
{
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
	cout << "long_name: " << mpFormatCtx->iformat->long_name << endl;
	cout << "name: " << mpFormatCtx->iformat->name << endl;

	mnVideoIndex = av_find_best_stream(mpFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, -1);
	mnAudioIndex = av_find_best_stream(mpFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, -1);
	mnWidth = mpFormatCtx->streams[mnVideoIndex]->codec->width;
	mnHeight = mpFormatCtx->streams[mnVideoIndex]->codec->height;
	cout << "mnWidth: " << mnWidth << endl;
	cout << "mnHeight: " << mnHeight << endl;
}
void XDemuxThread::Start()
{
	mMutex.lock();
	if (!mpVideoThread)
	{
		mpVideoThread = new XVideoThread;
	}
	QThread::start();
	mpVideoThread->start();
	mMutex.unlock();
}

void XDemuxThread::pauseThread()
{
	mbIsPause = !mbIsPause;
	if (mpVideoThread)
	{
		mpVideoThread->mbIsPause = !mpVideoThread->mbIsPause;
	}
}

void XDemuxThread::StopThread()
{
	mbIsExit = true;
	if (mpVideoThread)
	{
		mpVideoThread->mbIsExit = true;
	}
}

void XDemuxThread::repeatPlay()
{
	mpXDemux->openMediaFile("E:/ffmpeg/video/sintel.h264");
	if (mpVideoThread)
	{
		mpVideoThread->open(mpXDemux->copyVideoParam(), mpVideoCall, mpXDemux->mnSrcWidth, mpXDemux->mnSrcHeight);
	}
}

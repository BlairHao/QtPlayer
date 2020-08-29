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
	mpAudioThread = NULL;
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
	if (mpAudioThread)
	{
		mpAudioThread->mbIsExit = true;
		mpAudioThread->wait();
		delete mpAudioThread;
		mpAudioThread = NULL;
	}
	mbIsExit = true;
	wait();
}

void XDemuxThread::run()
{
	while (!mbIsExit)
	{
		mMutex.lock();
		if (mbIsPause)
		{
			mMutex.unlock();
			msleep(5);
			continue;
		}
		if (!mpXDemux)
		{
			mMutex.unlock();
			msleep(5);
			continue;
		}
		
		if (!mpXDemux->mbRepeatPlay)
		{
			//repeatPlay();
		}
		//音视频同步
		if (mpVideoThread && mpAudioThread)
		{
			pts = mpAudioThread->pts;
			mpVideoThread->synpts = mpAudioThread->pts;
		}

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
			if (mpAudioThread) mpAudioThread->Push(pkt);
		}

		mMutex.unlock();
		msleep(1);
	}
	cout << "jkhdfjkshfjk" << endl;
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
	if (!mpAudioThread)
	{
		mpAudioThread = new XAudioThread;
	}
	mpVideoCall = pCall;
	m_strFilePath = strFilePath;
	mpXDemux->openMediaFile(strFilePath);
	mpVideoThread->open(mpXDemux->copyVideoParam(), mpVideoCall, mpXDemux->mnSrcWidth, mpXDemux->mnSrcHeight);
	mpAudioThread->open(mpXDemux->copyAudioParam(), mpXDemux->mnSampleRate, mpXDemux->mnChannels);
	//总时长(毫秒)
	mlTotalMs = mpXDemux->mlTotalMs;
	cout << "mlTotalMs: " << mlTotalMs << endl;
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
	if (!mpAudioThread)
	{
		mpAudioThread = new XAudioThread;
	}
	QThread::start();
	mpVideoThread->start();
	mpAudioThread->start();
	mMutex.unlock();
}

void XDemuxThread::pauseThread()
{
	mbIsPause = !mbIsPause;
	if (mpVideoThread)
	{
		mpVideoThread->mbIsPause = !mpVideoThread->mbIsPause;
	}
	if (mpAudioThread)
	{
		mpAudioThread->mbIsPause = !mpAudioThread->mbIsPause;
	}
}

void XDemuxThread::StopThread()
{
	mbIsExit = true;
	if (mpVideoThread)
	{
		mpVideoThread->mbIsExit = true;
	}
	if (mpAudioThread)
	{
		mpAudioThread->mbIsExit = true;
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

void XDemuxThread::clear()
{
	mMutex.lock();
	if (mpXDemux)
	{
		mpXDemux->clear();
	}
	if (mpVideoThread)
	{
		mpVideoThread->clear();
	}
	if (mpAudioThread)
	{
		mpAudioThread->clear();
	}
	mMutex.unlock();
}

void XDemuxThread::seek(double nPosition)
{
	clear();

	mMutex.lock();
	bool status = this->mbIsPause;
	mMutex.unlock();

	pauseThread();
	
	mMutex.lock();
	if (mpXDemux)
	{
		mpXDemux->seek(nPosition);
	}
	//实际要显示的pts
	long long seekPos = nPosition * mpXDemux->mlTotalMs;
	cout << "seekPos: " << seekPos;
	while (!mbIsExit)
	{
		AVPacket *pkt = mpXDemux->readVideo();
		if (!pkt)
		{
			break;
		}
		//如果解码到seekpts
		if (mpVideoThread->repaintPts(pkt,seekPos))
		{
			this->pts = seekPos;
			break;
		}
	}
	mMutex.unlock();

	//seek是非暂停状态
	if (!status)
	{
		pauseThread();
	}
}


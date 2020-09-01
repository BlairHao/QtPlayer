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

bool XDemuxThread::openMediaFile(const char *strFilePath, IVideoCall *pCall)
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
	bool bRet = mpXDemux->openMediaFile(strFilePath);
	if (!bRet)
	{
		mMutex.unlock();
		return false;
	}
	bRet = mpVideoThread->open(mpXDemux->copyVideoParam(), mpVideoCall, mpXDemux->mnSrcWidth, mpXDemux->mnSrcHeight);
	bRet = mpAudioThread->open(mpXDemux->copyAudioParam(), mpXDemux->mnSampleRate, mpXDemux->mnChannels);
	//总时长(毫秒)
	mlTotalMs = mpXDemux->mlTotalMs;
	mnWidth = mpXDemux->mnSrcWidth;
	mnHeight = mpXDemux->mnSrcHeight;
	cout << "mlTotalMs: " << mlTotalMs << endl;
	mMutex.unlock();
	return bRet;
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

void XDemuxThread::pauseThread(bool bIsPause)
{
	mbIsPause = bIsPause;
	if (mpVideoThread)
	{
		mpVideoThread->mbIsPause = bIsPause;
	}
	if (mpAudioThread)
	{
		mpAudioThread->mbIsPause = bIsPause;
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

	pauseThread(true);
	
	if (mpXDemux)
	{
		mpXDemux->seek(nPosition);
	}
	//实际要显示的pts
	long long seekPos = nPosition * mpXDemux->mlTotalMs;
	cout << "seekPos: " << seekPos<<endl;
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

	//seek是非暂停状态
	if (mbIsPause)
	{
		pauseThread(false);
	}
	mMutex.unlock();
}


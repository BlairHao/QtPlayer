#include "XAudioThread.h"
#include <iostream>
using namespace std;


XAudioThread::XAudioThread()
{
	mpResample = NULL;
	mpAudioPlay = NULL;
	pDecode = NULL;
	mpAudioPlay = XAudioPlay::Get();
	mpResample = new XResample;
	pDecode = new Decode;
}


XAudioThread::~XAudioThread()
{
	if (pDecode)
	{
		delete pDecode;
		pDecode = NULL;
	}
}

void XAudioThread::run()
{
	unsigned char *pcm = new unsigned char[1024 * 1024 * 10];
	while (!mbIsExit)
	{
		mMutex.lock();
		if (mbIsPause)
		{
			mMutex.unlock();
			msleep(5);
			continue;
		}
		AVPacket *pkt = Pop();
		bool bFlag = pDecode->Send(pkt);
		if (!bFlag)
		{
			mMutex.unlock();
			msleep(1);
			continue;
		}
		while (!mbIsExit)
		{
			AVFrame *pFrame = pDecode->Recv();
			if (!pFrame)
			{
				break;
			}
			//减去缓冲中未播放的时间
			pts = pDecode->mlPts - mpAudioPlay->GetNoPlayMs();
			int nSize = mpResample->resample(pFrame, pcm);
			while (!mbIsExit)
			{
				if (nSize <= 0)
				{
					break;
				}
				if (mpAudioPlay->getFree() < nSize || mbIsPause)
				{
					msleep(1);
					continue;
				}
				mpAudioPlay->write(pcm, nSize);
				break;
			}
		}
		//msleep(10);
		mMutex.unlock();
	}
	delete pcm;
}

void XAudioThread::close()
{
	//DecodeThread::close();
	if (!mpResample)
	{
		mpResample->close();
		mMutex.lock();
		delete mpResample;
		mpResample = NULL;
		mMutex.unlock();
	}
	if (mpAudioPlay)
	{
		mpAudioPlay->close();
		mMutex.lock();
		mpAudioPlay = NULL;
		mMutex.unlock();
	}
}

void XAudioThread::clear()
{
	//DecodeThread::clear();
	mMutex.lock();
	if (mpAudioPlay)
	{
		mpAudioPlay->clear();
	}
	mMutex.unlock();
}

void XAudioThread::setPause(bool isPause)
{
	this->mbIsPause = isPause;
	if (mpAudioPlay)
	{
		mpAudioPlay->setPause(isPause);
	}
}

bool XAudioThread::open(AVCodecContext *pCodecCtx, int nSampleRate, int nChannels)
{
	if (!pCodecCtx)
	{
		return false;
	}
	clear();
	mMutex.lock();
	pts = 0;
	bool bFlag = true;
	if (!mpResample->open(pCodecCtx))
	{
		cout << "XResample open failed!" << endl;
		bFlag = false;
	}
	mpAudioPlay->mnSampleRate = nSampleRate;
	mpAudioPlay->mnChannels = nChannels;
	if (!mpAudioPlay->open())
	{
		bFlag = false;
		cout << "XAudioPlay open failed!" << endl;
	}
	pDecode->openCodecParam(pCodecCtx);
	mMutex.unlock();
	return bFlag;
}

#include "XVideoThread.h"



XVideoThread::XVideoThread()
{
	mpCodecCtx = NULL;
	pVideoCall = NULL;
	pDecode = NULL;
	pDecode = new Decode;
}


XVideoThread::~XVideoThread()
{
	if (pDecode)
	{
		delete pDecode;
		pDecode = NULL;
	}
}

void XVideoThread::open(AVCodecContext *pCodecCtx, IVideoCall *pCall, int nWidth, int nHeight)
{
	if (!pCodecCtx)
	{
		return;
	}
	synpts = 0;
	mpCodecCtx = pCodecCtx;
	pVideoCall = pCall;
	pVideoCall->Init(nWidth, nHeight);
	pDecode->openCodecParam(mpCodecCtx);
}

void XVideoThread::run()
{
	while (!mbIsExit)
	{
		m_mutex.lock();
		if (mbIsPause)
		{
			m_mutex.unlock();
			continue;
		}
		//ÒôÊÓÆµÍ¬²½
		if (synpts > 0 && synpts < pDecode->mlPts)
		{
			m_mutex.unlock();
			msleep(1);
			continue;
		}
		AVPacket *pkt = Pop();
		bool bret = pDecode->Send(pkt);
		if (!bret)
		{
			m_mutex.unlock();
			msleep(1);
			continue;
		}
		while (!mbIsExit)
		{
			AVFrame *pframe = pDecode->Recv();
			if (!pframe) break;
			if (pVideoCall)
			{
				pVideoCall->Repaint(pframe);
				//msleep(40);
			}
		}
		m_mutex.unlock();
	}
}


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

bool XVideoThread::open(AVCodecContext *pCodecCtx, IVideoCall *pCall, int nWidth, int nHeight)
{
	if (!pCodecCtx)
	{
		return false;
	}
	clear();
	m_mutex.lock();
	synpts = 0;
	mpCodecCtx = pCodecCtx;
	pVideoCall = pCall;
	pVideoCall->Init(nWidth, nHeight);
	m_mutex.unlock();
	return pDecode->openCodecParam(mpCodecCtx);
}

void XVideoThread::clear()
{
	m_mutex.lock();
	pDecode->clear();
	m_mutex.unlock();
	DecodeThread::clear();
}

void XVideoThread::close()
{

}

bool XVideoThread::repaintPts(AVPacket *pkt, long long seekPts)
{
	m_mutex.lock();
	bool bRet = pDecode->Send(pkt);
	if (!bRet)
	{
		m_mutex.unlock();
		return true;
	}
	AVFrame *frame = pDecode->Recv();
	if (!frame)
	{
		m_mutex.unlock();
		return false;
	}
	//到达位置
	if (pDecode->mlPts >= seekPts)
	{
		if (pVideoCall)
		{
			pVideoCall->Repaint(frame);
		}
		m_mutex.unlock();
		return true;
	}
	freeFrame(&frame);
	m_mutex.unlock();
	return false;
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
		//音视频同步
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
				//msleep(1);
			}
		}
		m_mutex.unlock();
	}
}


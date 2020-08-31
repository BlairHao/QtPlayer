#include "DecodeThread.h"
extern "C"
{
#include "libavcodec/avcodec.h"
}


DecodeThread::DecodeThread()
{
	mbIsExit = false;
	mbIsPause = false;
	m_packetList.clear();
	mnMax = 100;
}


DecodeThread::~DecodeThread()
{
	close();
}

AVPacket *DecodeThread::Pop()
{
	m_mutex.lock();
	if (m_packetList.empty())
	{
		m_mutex.unlock();
		return NULL;
	}
	AVPacket *pkt = m_packetList.front();
	m_packetList.pop_front();
	m_mutex.unlock();
	return pkt;

}

bool DecodeThread::Push(AVPacket *pkt)
{
	if (!pkt)
	{
		return false;
	}
	while (!mbIsExit)
	{
		m_mutex.lock();
		if (m_packetList.size() < mnMax)
		{
			m_packetList.push_back(pkt);
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
		msleep(1);
	}
	return true;
}

void DecodeThread::clear()
{
	m_mutex.lock();
	while (!m_packetList.empty())
	{
		AVPacket *pkt = m_packetList.front();
		freePacket(&pkt);
		m_packetList.pop_front();
	}
	m_mutex.unlock();
}

void DecodeThread::close()
{
	//clear();

	//等待线程退出
	mbIsExit = true;
	wait();
}

void DecodeThread::freePacket(AVPacket **pkt)
{
	if (!pkt || !(*pkt))return;
	av_packet_free(pkt);
}

void DecodeThread::freeFrame(AVFrame **frame)
{
	if (!frame || !(*frame))return;
	av_frame_free(frame);
}

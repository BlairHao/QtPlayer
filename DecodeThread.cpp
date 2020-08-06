#include "DecodeThread.h"



DecodeThread::DecodeThread()
{
	mbIsExit = false;
	mbIsPause = false;
	m_packetList.clear();
	mnMax = 100;
}


DecodeThread::~DecodeThread()
{
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
	}
	return true;
}

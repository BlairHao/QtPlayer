#include "XAudioPlay.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <mutex>

class CXAudioPlay : public XAudioPlay
{
public:
	QAudioOutput *output = NULL;
	QIODevice *io = NULL;
	std::mutex mMutex;

	virtual long long GetNoPlayMs()
	{
		mMutex.lock();
		if (!output)
		{
			mMutex.unlock();
			return 0;
		}
		long long pts = 0;
		//还未播放的字节数
		double size = output->bufferSize() - output->bytesFree();
		//一秒音频字节数大小
		double secSize = mnSampleRate*(mnSampleSize / 8)*mnChannels;
		if (secSize <= 0)
		{
			pts = 0;
		}
		else
		{
			pts = (size / secSize) * 1000;
		}
		mMutex.unlock();
		return pts;
	}
	virtual void clear()
	{
		mMutex.lock();
		if (io)
		{
			io->reset();
		}
		mMutex.unlock();
	}

	virtual void close()
	{
		mMutex.lock();
		if (io)
		{
			io->close();
			io = NULL;
		}
		if (output)
		{
			output->stop();
			delete output;
			output = NULL;
		}
		mMutex.unlock();
	}

	virtual bool open()
	{
		close();
		QAudioFormat fmt;
		fmt.setSampleRate(mnSampleRate);
		fmt.setSampleSize(mnSampleSize);
		fmt.setChannelCount(mnChannels);
		fmt.setCodec("audio/pcm");
		fmt.setByteOrder(QAudioFormat::LittleEndian);
		fmt.setSampleType(QAudioFormat::UnSignedInt);
		mMutex.lock();
		output = new QAudioOutput(fmt);
		io = output->start();
		mMutex.unlock();
		if (io)
		{
			return true;
		}
		return false;
	}

	virtual void setPause(bool isPause)
	{
		mMutex.lock();
		if (!output)
		{
			mMutex.unlock();
			return;
		}
		if (isPause)
		{
			output->suspend();
		}
		else
		{
			output->resume();
		}
		mMutex.unlock();
	}

	virtual bool write(const unsigned char *data, int datasize)
	{
		if (!data || datasize <= 0)
		{
			return false;
		}
		mMutex.lock();
		if (!output || !io)
		{
			mMutex.unlock();
			return false;
		}
		int size = io->write((char *)data, datasize);
		mMutex.unlock();
		if (datasize != size)
		{
			return false;
		}
		return true;
	}

	virtual int getFree()
	{
		mMutex.lock();
		if (!output)
		{
			mMutex.unlock();
			return 0;
		}
		int free = output->bytesFree();
		mMutex.unlock();
		return free;
	}
};

XAudioPlay *XAudioPlay::Get()
{
	static CXAudioPlay play;
	return &play;
}


XAudioPlay::XAudioPlay()
{
}


XAudioPlay::~XAudioPlay()
{
}

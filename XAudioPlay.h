#pragma once
class XAudioPlay
{
public:
	XAudioPlay();
	~XAudioPlay();

	int mnSampleRate = 44100;
	int mnSampleSize = 16;
	int mnChannels = 2;

	//打开音频播放
	virtual bool open() = 0;
	virtual void close() = 0;
	virtual void clear() = 0;

	//返回缓冲中还没有播放的时间(毫秒)
	virtual long long GetNoPlayMs() = 0;
	//播放音频
	virtual bool write(const unsigned char *data, int datasize) = 0;
	virtual int getFree() = 0;
	virtual void setPause(bool isPause) = 0;

	static XAudioPlay *Get();
};


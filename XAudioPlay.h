#pragma once
class XAudioPlay
{
public:
	XAudioPlay();
	~XAudioPlay();

	int mnSampleRate = 44100;
	int mnSampleSize = 16;
	int mnChannels = 2;

	//����Ƶ����
	virtual bool open() = 0;
	virtual void close() = 0;
	virtual void clear() = 0;

	//���ػ����л�û�в��ŵ�ʱ��(����)
	virtual long long GetNoPlayMs() = 0;
	//������Ƶ
	virtual bool write(const unsigned char *data, int datasize) = 0;
	virtual int getFree() = 0;
	virtual void setPause(bool isPause) = 0;

	static XAudioPlay *Get();
};


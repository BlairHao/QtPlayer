#include "XResample.h"
#include <iostream>
using namespace std;
extern "C"
{
#include "libswresample/swresample.h"
#include "libavcodec/avcodec.h"
}
//#pragma comment(lib,"swresample.lib")

XResample::XResample()
{
}


XResample::~XResample()
{
}

bool XResample::open(AVCodecContext *param)
{
	if (!param)
	{
		return false;
	}
	mMutex.lock();
	//���mpSwrCtxΪNULL�����ռ�
	mpSwrCtx = 0;
	mpSwrCtx = swr_alloc_set_opts(mpSwrCtx,
		av_get_default_channel_layout(2),//�����ʽ
		(AVSampleFormat)outFormat,       //���������ʽ 1 AV_SAMPLE_FMT_S16
		param->sample_rate,              //���������
		av_get_default_channel_layout(param->channels),//�����ʽ
		(AVSampleFormat)param->sample_fmt,
		param->sample_rate,
		0, 0
	);
	
	int nRet = swr_init(mpSwrCtx);
	mMutex.unlock();
	if (nRet != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(nRet, buf, sizeof(buf) - 1);
		cout << "swr_init failed! : " << buf << endl;
		return false;
	}
	return true;
}

int XResample::resample(AVFrame *indata, unsigned char *d)
{
	if (!indata)
	{
		return 0;
	}
	if (!d)
	{
		av_frame_free(&indata);
		return 0;
	}
	uint8_t *data[2] = { 0 };
	data[0] = d;
	int nRet = swr_convert(mpSwrCtx,
		data, indata->nb_samples, //���
		(const uint8_t **)indata->data, indata->nb_samples //����
	);
	int outSize = nRet * indata->channels * av_get_bytes_per_sample((AVSampleFormat)outFormat);
	av_frame_free(&indata);
	if (nRet <= 0)
	{
		return nRet;
	}
	return outSize;
}
void XResample::close()
{
	mMutex.lock();
	if (mpSwrCtx)
	{
		swr_free(&mpSwrCtx);
	}
	mMutex.unlock();
}

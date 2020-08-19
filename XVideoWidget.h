#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include "IVideoCall.h"
#include <mutex>


class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions, public IVideoCall
{
	Q_OBJECT

public:
	XVideoWidget(QWidget *parent);
	~XVideoWidget();

	virtual void Init(int width, int height);

	//���ܳɹ�����ͷ�frame�ռ�
	virtual void Repaint(AVFrame *frame);
	void pictureScale(int nWidth, int nHeight);
protected:
	//ˢ����ʾ
	void paintGL();
	//��ʼ��gl
	void initializeGL();
	//���ڳߴ�仯
	void resizeGL(int width, int height);

signals:
	//void 
private:
	//shader����
	QGLShaderProgram program;

	//shader��yuv������ַ
	GLuint unis[3] = { 0 };
	//opengl��texture��ַ
	GLuint texs[3] = { 0 };

	unsigned char *datas[3] = { 0 };

	int mnWidth = 640;
	int mnHeight = 360;
	std::mutex mMutex;
	int           m_iMag;

};

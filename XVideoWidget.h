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

	//不管成功与否都释放frame空间
	virtual void Repaint(AVFrame *frame);
	void pictureScale(int nWidth, int nHeight);
protected:
	//刷新显示
	void paintGL();
	//初始化gl
	void initializeGL();
	//窗口尺寸变化
	void resizeGL(int width, int height);

signals:
	//void 
private:
	//shader程序
	QGLShaderProgram program;

	//shader中yuv变量地址
	GLuint unis[3] = { 0 };
	//opengl的texture地址
	GLuint texs[3] = { 0 };

	unsigned char *datas[3] = { 0 };

	int mnWidth = 640;
	int mnHeight = 360;
	std::mutex mMutex;
	int           m_iMag;

};

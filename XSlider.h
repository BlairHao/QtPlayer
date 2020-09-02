#pragma once
#include <QSlider>
#include <QMouseEvent>
class XSlider : public QSlider
{
	Q_OBJECT
public:
	XSlider(QWidget *parent = 0);
	~XSlider();

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);

signals:
	void sliderPressed(int);
	void sliderMoved(int);
};


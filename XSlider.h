#pragma once
#include <QSlider>
#include <QMouseEvent>
class XSlider : public QSlider
{
public:
	XSlider(QWidget *parent = 0);
	~XSlider();

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
};


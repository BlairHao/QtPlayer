#include "XSlider.h"



XSlider::XSlider(QWidget *parent)
	:QSlider(parent)
{
	/*this->setStyleSheet("QSlider{max-width:122;min-width:122;border-image: url();}\
											 QSlider::groove:horizontal{background-image: url(:/slider/Resources/soundsliderbk.png);height:18px;}\
											 QSlider::sub-page:horizontal{background-image: url(:/slider/Resources/soundsliderpos.png);border: 5px solid transparent;border-radius: 5px;}\
											 QSlider::add-page:horizontal{background-color: transparent;}\
											 QSlider::handle:horizontal{background-image: url(:/slider/Resources/soundsliderselect.png);width:32px;height:32px;margin-left: -5px;margin-top: -8px;margin-right: -5px;margin-bottom: -7px;}");
											 */
}


XSlider::~XSlider()
{
}

void XSlider::mousePressEvent(QMouseEvent *e)
{
	double ratio = (double)e->pos().x() / (double)width();
	setValue(ratio*maximum());
	QSlider::sliderPressed();
}

void XSlider::mouseReleaseEvent(QMouseEvent *e)
{
	QSlider::sliderReleased();
}

void XSlider::mouseMoveEvent(QMouseEvent *e)
{
	double ratio = (double)e->pos().x() / (double)width();
	setValue(ratio*maximum());
	QSlider::sliderMoved(ratio*maximum());
}

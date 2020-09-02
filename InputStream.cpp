#include "InputStream.h"



InputStream::InputStream(QWidget *parent)
	:QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(startPlay()));
}


InputStream::~InputStream()
{
}

void InputStream::startPlay()
{
	QString qstrStream = ui.lineEdit->text();
	if (qstrStream.isEmpty())
	{
		return;
	}
	emit startPlaySignal(qstrStream);
}

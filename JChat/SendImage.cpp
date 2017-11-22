#include "SendImage.h"
#include <QEventLoop>


JChat::SendImage::SendImage(QWidget *parent)
	: QWidget(parent,Qt::Window)
{
	ui.setupUi(this);
	setWindowModality(Qt::ApplicationModal);
	setWindowFlags(Qt::Window | Qt::WindowType::CustomizeWindowHint | Qt::WindowType::WindowCloseButtonHint);

	this->setFixedSize(size());
}

JChat::SendImage::~SendImage()
{
}

int
JChat::SendImage::exec(QImage const& image, QWidget * parent)
{
	QEventLoop el;
	SendImage widget(parent);
	widget.show();

	connect(widget.ui.btnOK, &QPushButton::clicked, [&]
	{
		el.exit(0);
	});

	connect(widget.ui.btnCancel, &QPushButton::clicked, [&]
	{
		el.exit(1);
	});
	connect(&widget, &SendImage::closed, [&]
	{
		el.exit(1);
	});

	const double w = widget.ui.label->width(), h = widget.ui.label->height();
	auto r0 = w / h;
	auto r1 = (double)image.width() / image.height();

	QImage img;
	if(r0 < r1)
	{
		img = image.scaledToWidth(w);
	}
	else
	{
		img = image.scaledToWidth(h);
	}

	widget.ui.label->setPixmap(QPixmap::fromImage(img));


	return el.exec();
}

void 
JChat::SendImage::closeEvent(QCloseEvent *event)
{
	emit closed();
}


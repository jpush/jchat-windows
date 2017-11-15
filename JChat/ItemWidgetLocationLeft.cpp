#include "ItemWidgetLocationLeft.h"


#include <QMouseEvent>
#include <QDesktopServices>
#include "Util.h"
#include "MessageListWidget.h"
#include "UserInfoWidget.h"

JChat::ItemWidgetLocationLeft::ItemWidgetLocationLeft(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);


	ui.labelImage->installEventFilter(this);

	connect(ui.label, &Label::clicked, this, [=]
	{
		if(_msg && _msg->sender)
		{
			auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
			UserInfoWidget::showUserInfo(listWidget->getClientObject(), _msg->sender, this->topLevelWidget());
		}
	});

}

JChat::ItemWidgetLocationLeft::~ItemWidgetLocationLeft()
{
}

void JChat::ItemWidgetLocationLeft::setImageHolder(QSize const& imageSize)
{
	auto sz = getImageThumbnailSize(imageSize);

	ui.labelImage->setScaledContents(true);
	ui.labelImage->setFixedSize(sz + QSize(10, 10));
}

void JChat::ItemWidgetLocationLeft::setImage(QPixmap const& img)
{
	auto sz = img.size();
	sz = getImageThumbnailSize(sz);
	img.scaledToWidth(sz.width(), Qt::TransformationMode::SmoothTransformation);

	ui.labelImage->setPixmap(img);
	ui.labelImage->setScaledContents(true);
	ui.labelImage->setFixedSize(sz + QSize(10, 10));
}

QSize JChat::ItemWidgetLocationLeft::itemWidgetSizeHint(QSize const& newSize)
{
	auto sz = newSize;
	sz.setHeight(ui.labelImage->height() + 30);
	return sz;
}

void JChat::ItemWidgetLocationLeft::setAvatarPixmap(QPixmap const& pixmap)
{
	ui.label->setPixmap(pixmap);
}

void JChat::ItemWidgetLocationLeft::setDisplayName(QString const& name)
{
	ui.labelName->setText(name);
}

void JChat::ItemWidgetLocationLeft::setComplete()
{
	ui.labelStatus->clear();
}

void JChat::ItemWidgetLocationLeft::setFailed()
{
	ui.labelStatus->setPixmap(getFailedPixmap());
}

void JChat::ItemWidgetLocationLeft::setProgress(int)
{
	ui.labelStatus->setMovie(getProgressMovie());
}

bool JChat::ItemWidgetLocationLeft::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == ui.labelImage)
	{
		if(event->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(event);
			if(ev->button() == Qt::LeftButton)
			{
				QString loc = "http://api.map.baidu.com/marker?location=%1,%2&title=%3&content=%3&output=html";
				loc = loc.arg(_location.latitude).arg(_location.longitude).arg(_location.label.c_str());
				QDesktopServices::openUrl(loc);
			}
		}
	}
	return false;
}

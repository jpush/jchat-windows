#include "ItemWidgetLocationRight.h"
#include <QMouseEvent>
#include <QDesktopServices>
#include "Util.h"
#include "MessageListWidget.h"
#include "UserInfoWidget.h"
namespace JChat {

	ItemWidgetLocationRight::ItemWidgetLocationRight(QWidget *parent)
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

	ItemWidgetLocationRight::~ItemWidgetLocationRight()
	{
	}


	void JChat::ItemWidgetLocationRight::setImageHolder(QSize const& imageSize)
	{
		auto sz = getImageThumbnailSize(imageSize);

		ui.labelImage->setScaledContents(true);
		ui.labelImage->setFixedSize(sz + QSize(10, 10));
	}

	void JChat::ItemWidgetLocationRight::setImage(QPixmap const& img)
	{
		auto sz = img.size();
		sz = getImageThumbnailSize(sz);
		img.scaledToWidth(sz.width(), Qt::TransformationMode::SmoothTransformation);

		ui.labelImage->setPixmap(img);
		ui.labelImage->setScaledContents(true);
		ui.labelImage->setFixedSize(sz + QSize(10, 10));
	}

	QSize JChat::ItemWidgetLocationRight::itemWidgetSizeHint(QSize const& newSize)
	{
		auto sz = newSize;
		sz.setHeight(ui.labelImage->height() + 10);
		return sz;
	}

	void JChat::ItemWidgetLocationRight::setAvatarPixmap(QPixmap const& pixmap)
	{
		ui.label->setPixmap(pixmap);
	}

	void JChat::ItemWidgetLocationRight::setComplete()
	{
		ui.labelStatus->clear();
	}

	void JChat::ItemWidgetLocationRight::setFailed()
	{
		ui.labelStatus->setPixmap(getFailedPixmap());
	}

	void JChat::ItemWidgetLocationRight::setProgress(int)
	{
		ui.labelStatus->setMovie(getProgressMovie());
	}

	void ItemWidgetLocationRight::setUnreadUserCount(size_t count)
	{
		auto text = getUnreadCountText(count);
		if(!text.isNull())
		{
			ui.labelUnread->setText(text);
		}
	}

	bool JChat::ItemWidgetLocationRight::eventFilter(QObject *watched, QEvent *event)
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

} // namespace JChat

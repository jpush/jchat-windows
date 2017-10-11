
#include "ItemWidgetImageRight.h"

#include <QDesktopServices>
#include <QMouseEvent>
#include "Util.h"
#include "MessageListWidget.h"
#include "UserInfoWidget.h"

namespace JChat
{
	ItemWidgetImageRight::ItemWidgetImageRight(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		ui.labelImage->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		connect(ui.labelImage, &QLabel::customContextMenuRequested, this, [=](QPoint const& pt)
		{
			auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
			auto menu = listWidget->createMenu(this);
			menu->exec(ui.labelImage->mapToGlobal(pt));
		});

		ui.labelImage->installEventFilter(this);

		ui.labelUnread->installEventFilter(this);
		ui.label->installEventFilter(this);

	}

	ItemWidgetImageRight::~ItemWidgetImageRight()
	{
	}

	void ItemWidgetImageRight::setImageHolder(QSize const& imageSize)
	{
		auto sz = getImageThumbnailSize(imageSize);

		ui.labelImage->setScaledContents(true);
		ui.labelImage->setFixedSize(sz + QSize(10, 10));
	}

	void ItemWidgetImageRight::setFilePath(QString const& filePath)
	{
		ui.labelImage->setProperty("filepath", filePath);
	}

	void ItemWidgetImageRight::setImage(QPixmap const& img)
	{
		auto sz = img.size();
		sz = getImageThumbnailSize(sz);
		auto image =img.scaledToWidth(sz.width(), Qt::TransformationMode::SmoothTransformation);

		ui.labelImage->setPixmap(image);
		ui.labelImage->setScaledContents(true);
		ui.labelImage->setFixedSize(sz + QSize(10, 10));
	}

	void ItemWidgetImageRight::setMovie(QMovie *mv, QSize const& sz)
	{
		auto newSz = getImageThumbnailSize(sz);
		mv->setScaledSize(newSz);

		mv->setParent(this);
		ui.labelImage->setMovie(mv);

		if (this->isVisible())
		{
			mv->start();
		}

		ui.labelImage->setScaledContents(true);
		ui.labelImage->setFixedSize(newSz + QSize(10, 10));
	}

	QSize ItemWidgetImageRight::itemWidgetSizeHint(QSize const& newSize)
	{
		auto sz = newSize;
		sz.setHeight(ui.labelImage->height() + 10);
		return sz;
	}

	void ItemWidgetImageRight::setComplete()
	{
		ui.labelStatus->clear();
	}

	void ItemWidgetImageRight::setFailed()
	{
		ui.labelStatus->setPixmap(getFailedPixmap());
	}

	void ItemWidgetImageRight::setProgress(int)
	{
		ui.labelStatus->setMovie(getProgressMovie());
	}


	void ItemWidgetImageRight::setUnreadUserCount(size_t count)
	{
		auto text = getUnreadCountText(count);
		if(!text.isNull())
		{
			ui.labelUnread->setText(text);
		}
	}

	void ItemWidgetImageRight::visibleChanged(bool isVisible)
	{
		auto mv = ui.labelImage->movie();
		if(mv)
		{
			mv->setPaused(!isVisible);
		}
	}

	bool ItemWidgetImageRight::eventFilter(QObject *watched, QEvent *event)
	{
		if(watched == ui.label && event->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(event);
			if(ev->button() == Qt::LeftButton)
			{
				if(_msg && _msg->sender)
				{
					auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
					UserInfoWidget::showUserInfo(listWidget->getClientObject(), _msg->sender, this->topLevelWidget());
				}
			}
		}
		if(watched == ui.labelImage && event->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(event);
			if(ev->button() == Qt::LeftButton)
			{
				auto filepath = ui.labelImage->property("filepath").toString();
				if(!filepath.isEmpty())
				{
					QDesktopServices::openUrl(QUrl::fromLocalFile(filepath));
				}
				else
				{

				}
			}
		}

		if(watched == ui.labelUnread&& event->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(event);
			if(ev->button() == Qt::LeftButton && _msg)
			{
				if(_msg->unreadUserCount&& _msg->groupId)
				{
					auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
					listWidget->showUnreadUsers(_msg->msgId);
				}
			}
		}
		return false;
	}

}
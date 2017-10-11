#include "ItemWidgetFileRight.h"

#include <QMouseEvent>
#include <QDesktopServices>
#include "MessageListWidget.h"
#include "UserInfoWidget.h"

namespace JChat
{

	ItemWidgetFileRight::ItemWidgetFileRight(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);
		ui.frameF->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		connect(ui.frameF, &QFrame::customContextMenuRequested, this, [=](QPoint const& pt)
		{
			auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
			auto menu = listWidget->createMenu(this);
			menu->exec(ui.frameF->mapToGlobal(pt));
		});

		ui.labelUnread->installEventFilter(this);
		ui.frameF->installEventFilter(this);

		ui.label->installEventFilter(this);

	}

	ItemWidgetFileRight::~ItemWidgetFileRight()
	{
	}

	void ItemWidgetFileRight::setFileName(QString const&fileName)
	{
		ui.labelFileName->setText(fileName);
	}

	void ItemWidgetFileRight::setFileSize(int64_t fileSize)
	{
		ui.labelFileSize->setText(QString("%1KB").arg(fileSize / 1000));
	}

	void ItemWidgetFileRight::setComplete()
	{
		ui.labelStatusText->setText(u8"ÒÑ·¢ËÍ");
		ui.progressBar->setValue(100);
		ui.labelStatus->clear();
	}

	void ItemWidgetFileRight::setFailed()
	{
		ui.labelStatusText->setText(u8"·¢ËÍÊ§°Ü");
		ui.labelStatus->setPixmap(getFailedPixmap());
	}

	void ItemWidgetFileRight::setProgress(int value)
	{
		ui.progressBar->setValue(value);
		ui.labelStatus->setMovie(getProgressMovie());

	}

	void ItemWidgetFileRight::setAvatarPixmap(QPixmap const& pixmap)
	{
		ui.label->setPixmap(pixmap);
	}


	void ItemWidgetFileRight::setUnreadUserCount(size_t count)
	{
		auto text = getUnreadCountText(count);
		if(!text.isNull())
		{
			ui.labelUnread->setText(text);
		}
	}

	QSize ItemWidgetFileRight::itemWidgetSizeHint(QSize const& newSize)
	{
		auto sz = newSize;
		sz.setHeight(ui.frameF->height() + 10);
		return sz;
	}

	bool ItemWidgetFileRight::eventFilter(QObject *watched, QEvent *event)
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
		if(watched == ui.labelUnread && event->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(event);
			if(ev->button() == Qt::LeftButton && _msg)
			{
				if(_msg->unreadUserCount && _msg->groupId)
				{
					auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
					listWidget->showUnreadUsers(_msg->msgId);
				}
			}
		}

		if(watched == ui.frameF && event->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(event);
			if(ev->button() == Qt::LeftButton)
			{
				auto filepath = ui.frameF->property("filepath").toString();
				if(!filepath.isEmpty())
				{
					QDesktopServices::openUrl(QUrl::fromLocalFile(filepath));
				}
			}
		}

		return false;
	}

}
#include "ItemWidgetUserCardRight.h"

#include <QMouseEvent>
#include "MessageListWidget.h"
#include "Util.h"
#include "UserInfoWidget.h"

namespace JChat {

	ItemWidgetUserCardRight::ItemWidgetUserCardRight(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		ui.frameF->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		connect(ui.frameF, &QLabel::customContextMenuRequested, this, [=](QPoint const& pt)
		{
			auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
			auto menu = listWidget->createMenu(this);
			menu->exec(ui.frameF->mapToGlobal(pt));
		});
		ui.labelUnread->installEventFilter(this);
		ui.frameF->installEventFilter(this);

		connect(ui.label, &Label::clicked, this, [=]
		{
			if(_msg && _msg->sender)
			{
				auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
				UserInfoWidget::showUserInfo(listWidget->getClientObject(), _msg->sender, this->topLevelWidget());
			}
		});

	}

	ItemWidgetUserCardRight::~ItemWidgetUserCardRight()
	{
	}

	JChat::ItemWidgetInterface::ItemFlags ItemWidgetUserCardRight::flags()
	{
		return usercard | outgoing;
	}

	QSize ItemWidgetUserCardRight::itemWidgetSizeHint(QSize const& newViewportSize)
	{
		auto sz = newViewportSize;
		sz.setHeight(ui.frameF->height() + 10);
		return sz;
	}

	void ItemWidgetUserCardRight::setAvatarPixmap(QPixmap const& pixmap)
	{
		ui.label->setPixmap(pixmap);
	}

	void ItemWidgetUserCardRight::setComplete()
	{
		ui.labelStatus->clear();
	}

	void ItemWidgetUserCardRight::setFailed()
	{
		ui.labelStatus->setPixmap(getFailedPixmap());
	}

	void ItemWidgetUserCardRight::setProgress(int)
	{
		ui.labelStatus->setMovie(getProgressMovie());
	}

	void ItemWidgetUserCardRight::setCardUserId(Jmcpp::UserId const& userId)
	{
		_userId = userId;
		auto name = QString::fromStdString(userId.username);
		ui.labelDisplayName->setText(name);
		ui.labelUserName->setText(name);
	}

	//
	void ItemWidgetUserCardRight::setCardUserInfo(Jmcpp::UserInfo const& info)
	{
		auto display = getUserDisplayName(info);
		ui.labelDisplayName->setText(display);
		ui.labelDisplayName->setToolTip(display);

		auto name = QString(u8"ÓÃ»§Ãû:") + info.userId.username.c_str();
		ui.labelUserName->setText(name);
		ui.labelUserName->setToolTip(name);
	}
	void ItemWidgetUserCardRight::setCardAvatar(QPixmap const& pixmap)
	{
		ui.labelAvatar->setPixmap(pixmap);
	}

	void ItemWidgetUserCardRight::setUnreadUserCount(size_t count)
	{
		auto text = getUnreadCountText(count);
		if(!text.isNull())
		{
			ui.labelUnread->setText(text);
		}
	}

	bool ItemWidgetUserCardRight::eventFilter(QObject *watched, QEvent *event)
	{
		if(watched == ui.labelUnread && event->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(event);
			if(ev->button() == Qt::LeftButton && _msg)
			{
				if(_msg->unreadUserCount && _msg->conId.isGroup())
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
				if(_userId)
				{
					auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
					UserInfoWidget::showUserInfo(listWidget->getClientObject(), _userId, this->topLevelWidget());
				}
			}
		}
	
		return false;
	}

} // namespace JChat

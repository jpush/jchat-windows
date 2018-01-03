#include "ItemWidgetUserCardLeft.h"

#include <QMouseEvent>
#include "MessageListWidget.h"
#include "Util.h"
#include "UserInfoWidget.h"
namespace JChat {

	ItemWidgetUserCardLeft::ItemWidgetUserCardLeft(QWidget *parent)
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

	ItemWidgetUserCardLeft::~ItemWidgetUserCardLeft()
	{
	}

	JChat::ItemWidgetInterface::ItemFlags ItemWidgetUserCardLeft::flags()
	{
		return usercard | incoming;
	}

	QSize ItemWidgetUserCardLeft::itemWidgetSizeHint(QSize const& newViewportSize)
	{
		auto sz = newViewportSize;
		sz.setHeight(ui.frameF->height() + 30);
		return sz;
	}

	void ItemWidgetUserCardLeft::setAvatarPixmap(QPixmap const& pixmap)
	{
		ui.label->setPixmap(pixmap);
	}

	void ItemWidgetUserCardLeft::setDisplayName(QString const& name)
	{
		ui.labelName->setText(name);
	}

	void ItemWidgetUserCardLeft::setComplete()
	{
		ui.labelStatus->clear();
	}

	void ItemWidgetUserCardLeft::setFailed()
	{
		ui.labelStatus->setPixmap(getFailedPixmap());
	}

	void ItemWidgetUserCardLeft::setProgress(int)
	{
		ui.labelStatus->setMovie(getProgressMovie());
	}

	//
	void ItemWidgetUserCardLeft::setCardUserId(Jmcpp::UserId const& userId)
	{
		_userId = userId;
		auto name = QString::fromStdString(userId.username);
		ui.labelDisplayName->setText(name);
		ui.labelUserName->setText(name);
	}

	void ItemWidgetUserCardLeft::setCardUserInfo(Jmcpp::UserInfo const& info)
	{
		auto display = getUserDisplayName(info);
		ui.labelDisplayName->setText(display);
		ui.labelDisplayName->setToolTip(display);

		auto name = QString(u8"用户名:") + info.userId.username.c_str();
		ui.labelUserName->setText(name);
		ui.labelUserName->setToolTip(name);
	}
	void ItemWidgetUserCardLeft::setCardAvatar(QPixmap const& pixmap)
	{
		ui.labelAvatar->setPixmap(pixmap);
	}

	bool ItemWidgetUserCardLeft::eventFilter(QObject *watched, QEvent *event)
	{
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

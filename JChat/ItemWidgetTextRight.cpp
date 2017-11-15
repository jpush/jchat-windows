
#include "ItemWidgetTextRight.h"

#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMovie>

#include "MessageListWidget.h"
#include "UnreadUsers.h"
#include "UserInfoWidget.h"

namespace JChat
{
	ItemWidgetTextRight::ItemWidgetTextRight(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		ui.textBrowser->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

		connect(ui.textBrowser, &QTextBrowser::customContextMenuRequested, this, [=](QPoint const& pt)
		{
			auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
			auto menu = listWidget->createMenu(this);
			menu->exec(ui.textBrowser->mapToGlobal(pt));
		});

		ui.textBrowser->installEventFilter(this);
		ui.labelUnread->installEventFilter(this);
		connect(ui.label, &Label::clicked, this, [=]
		{
			if(_msg && _msg->sender)
			{
				auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
				UserInfoWidget::showUserInfo(listWidget->getClientObject(), _msg->sender, this->topLevelWidget());
			}
		});

	}

	ItemWidgetTextRight::~ItemWidgetTextRight()
	{
	}

	void ItemWidgetTextRight::setHtml(QString const& html)
	{
		ui.textBrowser->setHtml(html);
	}

	QSize ItemWidgetTextRight::itemWidgetSizeHint(QSize const& newSize)
	{
		QTextDocument doc;
		doc.setDefaultFont(ui.textBrowser->font());
		doc.setDefaultTextOption(ui.textBrowser->document()->defaultTextOption());

		doc.setTextWidth(newSize.width() - 200);

		doc.setHtml(ui.textBrowser->toHtml());

		auto sz = doc.size().toSize();
		auto w = doc.idealWidth();

		ui.textBrowser->setMinimumSize(w + 20, sz.height() + 20);
		ui.textBrowser->setMaximumSize(w + 20, sz.height() + 20);

		auto result = newSize;
		result.setHeight(sz.height() + 25);
		return result;
	}


	void ItemWidgetTextRight::setAvatarPixmap(QPixmap const& pixmap)
	{
		ui.label->setPixmap(pixmap);
	}

	void ItemWidgetTextRight::setComplete()
	{
		ui.labelStatus->clear();
	}

	void ItemWidgetTextRight::setFailed()
	{
		ui.labelStatus->setPixmap(getFailedPixmap());
	}

	void ItemWidgetTextRight::setProgress(int)
	{
		ui.labelStatus->setMovie(getProgressMovie());
	}

	void ItemWidgetTextRight::setUnreadUserCount(size_t count)
	{
		if(!_msg || !_msg->msgId)
		{
			return;
		}
		auto text = getUnreadCountText(count);
		if(!text.isNull())
		{
			ui.labelUnread->setText(text);
		}
	}

	bool ItemWidgetTextRight::eventFilter(QObject *watched, QEvent *event)
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

		if(watched == ui.textBrowser)
		{
			if(event->type() == QEvent::MouseButtonPress)
			{
				auto ev = static_cast<QMouseEvent*>(event);
				if(ev->button() == Qt::RightButton)
				{
					ui.textBrowser->setProperty("pressed", true);
					ui.textBrowser->style()->polish(ui.textBrowser);
				}
			}
			else if(event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::Leave)
			{
				auto ev = static_cast<QMouseEvent*>(event);
				ui.textBrowser->setProperty("pressed", false);
				ui.textBrowser->style()->polish(ui.textBrowser);
			}
		}
		return false;
	}


}
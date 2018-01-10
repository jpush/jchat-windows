#include "ItemWidgetTextLeft.h"

#include <QMouseEvent>
#include "MessageListWidget.h"
#include "UserInfoWidget.h"

namespace JChat
{
	ItemWidgetTextLeft::ItemWidgetTextLeft(QWidget *parent)
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

		connect(ui.label, &Label::clicked, this, [=]
		{
			if(_msg && _msg->sender)
			{
				auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
				UserInfoWidget::showUserInfo(listWidget->getClientObject(), _msg->sender, this->topLevelWidget());
			}
		});

	}

	ItemWidgetTextLeft::~ItemWidgetTextLeft()
	{

	}

	void ItemWidgetTextLeft::setHtml(QString const& html)
	{
		ui.textBrowser->setHtml(html);
	}

	QSize ItemWidgetTextLeft::itemWidgetSizeHint(QSize const& newSize)
	{
		QTextDocument doc;
		doc.setDefaultFont(ui.textBrowser->font());
		doc.setDefaultTextOption(ui.textBrowser->document()->defaultTextOption());
		doc.setTextWidth(newSize.width() - 200);
		doc.setHtml(ui.textBrowser->toHtml());

		auto sz = doc.size().toSize();

		auto w = doc.idealWidth();

		ui.textBrowser->setMinimumSize(w + 22, sz.height() + 20);
		ui.textBrowser->setMaximumSize(w + 22, sz.height() + 20);

		auto result = newSize;
		result.setHeight(sz.height() + 45);
		return result;
	}

	void ItemWidgetTextLeft::setDisplayName(QString const& name)
	{
		ui.labelName->setText(name);
	}

	void ItemWidgetTextLeft::setAvatarPixmap(QPixmap const& pixmap)
	{
		ui.label->setPixmap(pixmap);
	}


	void ItemWidgetTextLeft::setComplete()
	{
		ui.labelStatus->clear();
	}

	void ItemWidgetTextLeft::setFailed()
	{
		ui.labelStatus->setPixmap(getFailedPixmap());
	}

	void ItemWidgetTextLeft::setProgress(int)
	{
		ui.labelStatus->setMovie(getProgressMovie());
	}

	bool ItemWidgetTextLeft::eventFilter(QObject *watched, QEvent *event)
	{
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


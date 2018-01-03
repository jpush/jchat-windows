#include "ItemWidgetFileLeft.h"

#include <QDesktopServices>
#include <QMouseEvent>
#include "MessageListWidget.h"
#include "UserInfoWidget.h"

namespace JChat {

	ItemWidgetFileLeft::ItemWidgetFileLeft(QWidget *parent)
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


		connect(ui.label, &Label::clicked, this, [=]
		{
			if(_msg && _msg->sender)
			{
				auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
				UserInfoWidget::showUserInfo(listWidget->getClientObject(), _msg->sender, this->topLevelWidget());
			}
		});


		ui.frameF->installEventFilter(this);
		connect(ui.btnFileAction, &QToolButton::clicked, this, &ItemWidgetFileLeft::downloadClicked);
	}

	ItemWidgetFileLeft::~ItemWidgetFileLeft()
	{
	}

	QSize ItemWidgetFileLeft::itemWidgetSizeHint(QSize const& newSize)
	{
		auto sz = newSize;
		sz.setHeight(ui.frameF->height() + 30);
		return sz;
	}

	void ItemWidgetFileLeft::setComplete()
	{
		ui.progressBar->setRange(0, 100);
		ui.progressBar->setValue(100);
		ui.btnFileAction->setText(u8"已下载");
		ui.btnFileAction->setEnabled(false);
	}

	void ItemWidgetFileLeft::setFailed()
	{
		ui.progressBar->setRange(0, 100);
		ui.progressBar->setValue(0);
		ui.btnFileAction->setText(u8"接收失败");
		ui.btnFileAction->setEnabled(true);
	}

	void ItemWidgetFileLeft::setProgress(int value)
	{
		//ui.progressBar->setValue(value);
		if(value == -1)
		{
			ui.progressBar->setRange(0, 0);
		}
		else
		{
			ui.progressBar->setValue(value);
		}

		ui.btnFileAction->setText(u8"正在下载");
		ui.btnFileAction->setEnabled(false);

		//ui.labelStatus->setMovie(getProgressMovie());
	}

	void ItemWidgetFileLeft::setAvatarPixmap(QPixmap const& pixmap)
	{
		ui.label->setPixmap(pixmap);
	}

	void ItemWidgetFileLeft::setDisplayName(QString const& name)
	{
		ui.labelName->setText(name);
	}

	bool ItemWidgetFileLeft::eventFilter(QObject *watched, QEvent *event)
	{

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

} // namespace JChat

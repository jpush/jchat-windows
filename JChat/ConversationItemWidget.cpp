#include "ConversationItemWidget.h"

#include <QDebug>
#include <QResizeEvent>
#include <QMenu>
namespace JChat {

	ConversationItemWidget::ConversationItemWidget(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		auto menu = new QMenu(this);
		auto act = menu->addAction(u8"会话置顶");
		act->setCheckable(true);

		menu->addAction(u8"删除会话", [=]
		{
			Q_EMIT closeClicked();
		});


		connect(menu, &QMenu::aboutToShow, this, [=]
		{
			act->setChecked(_sticktop);
		});

		connect(act, &QAction::triggered, this, [=](bool checked)
		{
			setSticktop(checked);
			Q_EMIT sticktopChanged(checked);
		});



		setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		connect(this, &QWidget::customContextMenuRequested, this, [=](QPoint const& pt)
		{
			menu->exec(this->mapToGlobal(pt));
		});

		_unreadCountLabel = new QLabel(this);
		_unreadCountLabel->setAlignment(Qt::AlignCenter);
		_unreadCountLabel->setGeometry(45, 10, 18, 18);
		_unreadCountLabel->setStyleSheet(R"(

QLabel
{
	border:none;
	color: rgb(255, 255, 255);
	background-color: rgb(255, 0, 0);
	border-radius:9px;
}

)");
		_unreadCountLabel->raise();
		_unreadCountLabel->hide();



		setNotDisturb(false);
	}

	ConversationItemWidget::~ConversationItemWidget()
	{
	}

	void ConversationItemWidget::on_btnClose_clicked()
	{
		Q_EMIT closeClicked();
	}

	void ConversationItemWidget::setNotDisturb(bool enabled)
	{
		_isNotDisturb = enabled;

		if(enabled)
		{
			_unreadCountLabel->setGeometry(55, 13, 8, 8);
			_unreadCountLabel->setStyleSheet(R"(

	QLabel
	{
		border:none;
		color: rgb(255, 255, 255);
		background-color: rgb(255, 0, 0);
		border-radius:4px;
	}

	)");

			_unreadCountLabel->setText("");
		}
		else
		{
			_unreadCountLabel->setGeometry(45, 10, 18, 18);
			_unreadCountLabel->setStyleSheet(R"(

	QLabel
	{
		border:none;
		color: rgb(255, 255, 255);
		background-color: rgb(255, 0, 0);
		border-radius:9px;
	}

	)");
			setUnreadMessageCount(_unreadCount);

		}
	}


	void ConversationItemWidget::setSticktop(bool enabled)
	{
		_sticktop = enabled;
		ui.frame->setProperty("sticktop", enabled);
		ui.frame->style()->polish(ui.frame);
	}

	void ConversationItemWidget::setUnreadMessageCount(int count)
	{
		_unreadCount = count;
		if(count <= 0)
		{
			_unreadCountLabel->hide();
			return;
		}
		_unreadCountLabel->show();
		_unreadCountLabel->raise();

		if(_isNotDisturb)
		{
			return;
		}

		if(count < 100)
		{
			_unreadCountLabel->setText(QString::number(count));
			_unreadCountLabel->resize(18, 18);
		}
		else
		{
			_unreadCountLabel->setText("99+");
			_unreadCountLabel->resize(30, 18);

		}
	}

	void ConversationItemWidget::enterEvent(QEvent *event)
	{
		ui.btnClose->setEnabled(true);
	}

	void ConversationItemWidget::leaveEvent(QEvent *event)
	{
		ui.btnClose->setEnabled(false);
	}


} // namespace JChat

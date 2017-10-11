#include "TrayMessageItemWidget.h"


JChat::TrayMessageItemWidget::TrayMessageItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

JChat::TrayMessageItemWidget::~TrayMessageItemWidget()
{
}

void JChat::TrayMessageItemWidget::setUnreadMessageCount(int count)
{
	_unreadCount = count;
	if(count <= 0)
	{
		ui.unreadCountLabel->hide();
		return;
	}
	ui.unreadCountLabel->show();
	ui.unreadCountLabel->raise();


	if(count < 100)
	{
		ui.unreadCountLabel->setText(QString::number(count));
	}
	else
	{
		ui.unreadCountLabel->setText("99+");
	}
}


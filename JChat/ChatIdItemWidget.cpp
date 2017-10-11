#include "ChatIdItemWidget.h"



JChat::ChatIdItemWidget::ChatIdItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

JChat::ChatIdItemWidget::~ChatIdItemWidget()
{
}

Q_SLOT 
void JChat::ChatIdItemWidget::on_checkBox_toggled(bool checked)
{
	Q_EMIT toggled(checked);
}

Q_SLOT 
void JChat::ChatIdItemWidget::on_btnClose_clicked()
{
	Q_EMIT closeClicked();
}


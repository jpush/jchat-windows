#include "ToolButton.h"

#include <QPainter>
#include <QLabel>

namespace JChat {

	ToolButton::ToolButton(QWidget *parent)
		: QToolButton(parent)
	{
		_unread = new QLabel(this);
		_unread->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Preferred);
		_unread->setAlignment(Qt::AlignCenter);
		_unread->setGeometry(36, 0, 16, 16);
		_unread->setStyleSheet(R"(
QLabel
{
	border:none;
	color: rgb(255, 255, 255);
	background-color: rgb(255, 0, 0);
	border-radius:8px;
	padding:0px 4px;
}

 )");
		_unread->raise();
		_unread->setAttribute(Qt::WA_TransparentForMouseEvents);

		_unread->hide();

		connect(this, &QToolButton::clicked, this, [=]
		{
			setCount(0);
		});

	}

	ToolButton::~ToolButton()
	{
	}

	void ToolButton::setCount(int count)
	{
		_count = count;
		if(_count)
		{
			_unread->setText(QString::number(_count));
			_unread->raise();
			_unread->show();
		}
		else
		{
			_unread->hide();
		}
	}



	void ToolButton::addCount(int count)
	{
		_count += count;
		if(_count)
		{
			_unread->setText(QString::number(_count));
			_unread->raise();
			_unread->show();
		}
		else
		{
			_unread->hide();
		}
	}

} // namespace JChat

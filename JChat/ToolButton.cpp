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
		_unread->setGeometry(20, 0, 16, 16);
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


	}

	ToolButton::~ToolButton()
	{
	}

	void ToolButton::setCount(int count)
	{
		_count = count;

		if(count > 999)
		{
			_unread->setText("...");
			_unread->resize(30, 16);
			_unread->raise();
			_unread->show();
		}
		else if(count > 99)
		{
			_unread->setText(QString::number(count));
			_unread->resize(30, 16);
			_unread->raise();
			_unread->show();
		}
		else if(count > 1)
		{
			_unread->setText(QString::number(count));
			_unread->resize(20, 16);
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
		setCount(_count);
	}

} // namespace JChat

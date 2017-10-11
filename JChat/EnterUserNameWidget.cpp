#include "EnterUserNameWidget.h"

#include <QPushButton>
#include <QEventLoop>

namespace JChat
{
	EnterUserNameWidget::EnterUserNameWidget(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);
		this->setWindowFlags(Qt::Window | Qt::WindowType::CustomizeWindowHint | Qt::WindowType::WindowCloseButtonHint);
		this->setFixedSize(size());
	}

	EnterUserNameWidget::~EnterUserNameWidget()
	{

	}

	void EnterUserNameWidget::closeEvent(QCloseEvent *event)
	{
		emit closed();

	}
	QString EnterUserNameWidget::getUserName(QWidget *parent, QString const& title)
	{
		EnterUserNameWidget w(parent);
		w.setWindowModality(Qt::ApplicationModal);

		if (!title.isEmpty())
		{
			w.setWindowTitle(title);
		}

		QEventLoop el;
		connect(w.ui.btnOK, &QPushButton::clicked, [&]
		{
			if(w.ui.lineEdit->text().isEmpty())
			{

			}
			else
			{
				el.exit(0);
			}
		});

		connect(w.ui.btnCancel, &QPushButton::clicked, [&]
		{
			el.exit(1);
		});

		connect(&w, &EnterUserNameWidget::closed, [&]
		{
			el.exit(1);
		});

		w.show();
		if(el.exec() == 0)
		{
			return w.ui.lineEdit->text();
		}

		return {};
	}

	

}
#include "RequestMessageWidget.h"
#include <QPushButton>
#include <QEventLoop>

namespace JChat{

	RequestMessageWidget::RequestMessageWidget(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);
		this->setWindowFlags(Qt::Window | Qt::WindowType::CustomizeWindowHint | Qt::WindowType::WindowCloseButtonHint);
		this->setFixedSize(size());
	}

	RequestMessageWidget::~RequestMessageWidget()
	{
	}

	QString RequestMessageWidget::getText(QWidget *parent /*= Q_NULLPTR*/)
	{
		RequestMessageWidget w(parent);
		w.setWindowModality(Qt::ApplicationModal);

		QEventLoop el;
		connect(w.ui.btnOK, &QPushButton::clicked, [&]
		{
			if(w.ui.textEdit->toPlainText().isEmpty())
			{
				w.ui.textEdit->setText(" ");
			}
			el.exit(0);
		});

		connect(w.ui.btnCancel, &QPushButton::clicked, [&]
		{
			el.exit(1);
		});
		connect(&w, &RequestMessageWidget::closed, [&]
		{
			el.exit(1);
		});

		w.show();
		if(el.exec() == 0)
		{
			return w.ui.textEdit->toPlainText();
		}

		return {};
	}

	void RequestMessageWidget::closeEvent(QCloseEvent *event)
	{
		emit closed();

	}

}
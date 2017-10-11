#pragma once

#include <QWidget>
#include "ui_EnterUserNameWidget.h"

namespace JChat
{

	class EnterUserNameWidget : public QWidget
	{
		Q_OBJECT

	public:
		EnterUserNameWidget(QWidget *parent = Q_NULLPTR);
		~EnterUserNameWidget();

		static QString getUserName(QWidget *parent = Q_NULLPTR, QString const& title = {});

	protected:
		virtual void closeEvent(QCloseEvent *event) override;
		Q_SIGNAL void closed();

	private:
		Ui::EnterUserNameWidget ui;
	};
}

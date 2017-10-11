#pragma once

#include <QWidget>
#include "ui_RequestMessageWidget.h"

namespace JChat
{
	class RequestMessageWidget : public QWidget
	{
		Q_OBJECT

	public:
		RequestMessageWidget(QWidget *parent = Q_NULLPTR);
		~RequestMessageWidget();

		static QString getText(QWidget *parent /*= Q_NULLPTR*/);
	protected:
		virtual void closeEvent(QCloseEvent *event) override;
		Q_SIGNAL void closed();

	private:
		Ui::RequestMessageWidget ui;
	};


}


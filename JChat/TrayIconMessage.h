#pragma once

#include <QWidget>
#include "ui_TrayIconMessage.h"
#include "ClientObject.h"

class QSystemTrayIcon;
namespace JChat
{
	class TrayIconMessage : public QWidget
	{
		Q_OBJECT

	public:
		TrayIconMessage(QSystemTrayIcon* tray, QWidget *parent = Q_NULLPTR);
		~TrayIconMessage();

		void show();

		Q_SIGNAL void cancelFlash();

		Q_SIGNAL void showMessage(Jmcpp::ConversationId const& conId);

	private:
		Ui::TrayIconMessage ui;

		QSystemTrayIcon* _tray;
	};
}

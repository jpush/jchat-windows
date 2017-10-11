#pragma once

#include <QWidget>
#include <QStandardItemModel>

#include "ui_UnreadUsers.h"

#include "ClientObject.h"
namespace JChat {

	class UnreadUsers : public QWidget
	{
		Q_OBJECT

	public:
		UnreadUsers(QWidget *parent = Q_NULLPTR);
		~UnreadUsers();

		static void showUnreadUsers(ClientObjectPtr const& co,int64_t msgId, QWidget* parent);

	protected:
		Q_SIGNAL void closed();

		virtual void closeEvent(QCloseEvent *event) override;


		pplx::task<void> updateUsers(ClientObjectPtr  co, int64_t msgId);

	private:
		Ui::UnreadUsers ui;


		QStandardItemModel* _modelUnread;
		QStandardItemModel* _modelRead;


	};

} // namespace JChat

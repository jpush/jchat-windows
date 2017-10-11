#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include "ui_SelectUserWidget.h"

#include "ClientObject.h"



namespace JChat
{
	class SelectUserWidget : public QWidget
	{
		Q_OBJECT

	public:
		SelectUserWidget(ClientObjectPtr const&co, bool onlyFriend = false, bool multiSelect = true, QWidget *parent = Q_NULLPTR);
		~SelectUserWidget();

		static std::vector<Jmcpp::ConversationId> getConversationIds(ClientObjectPtr const&co, QWidget* parent = nullptr, bool onlyFriend = false, bool multiSelect = true);

	protected:
		std::vector<Jmcpp::ConversationId> _getConversationIds() const;

		virtual void closeEvent(QCloseEvent *event) override;

		Q_SIGNAL void closed();
	private:
		Ui::SelectUserWidget ui;

		ClientObjectPtr _co;
	};

}
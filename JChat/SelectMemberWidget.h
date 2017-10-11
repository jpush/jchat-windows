#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include <QStyledItemDelegate>


#include "ui_SelectMemberWidget.h"

#include "ClientObject.h"

namespace JChat
{
	class SelectMemberWidget : public QWidget
	{
		Q_OBJECT

	public:
		SelectMemberWidget(ClientObjectPtr const&co, QWidget *parent = Q_NULLPTR);
		~SelectMemberWidget();


		static std::optional<std::vector<Jmcpp::UserId>> getUserIds(ClientObjectPtr const&co, QString const& title,
																	QString* groupName,
																	QWidget* parent = nullptr);

	protected:
		Q_SIGNAL void closed();

		std::vector<Jmcpp::UserId>	_getUserIds();

		void setGroupNameEnable(bool enabled)
		{
			ui.frameName->setVisible(enabled);
		}


		virtual void closeEvent(QCloseEvent *event) override;

	private:
		Ui::SelectMemberWidget ui;
		ClientObjectPtr _co;
	};



}


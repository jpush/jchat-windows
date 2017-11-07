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


		static std::optional<std::vector<Jmcpp::UserId>> getUserIds(ClientObjectPtr const&co, QString const& title, QString& groupName,
																	QWidget* parent = nullptr);



		static std::optional<std::vector<Jmcpp::UserId>> getUserIds(ClientObjectPtr const&co, QString const& title,
																	QString& groupName, QString & groupAvatar, bool& isPublic,
																	QWidget* parent = nullptr);



	protected:
		virtual bool eventFilter(QObject *watched, QEvent *event) override;
		virtual void closeEvent(QCloseEvent *event) override;

		Q_SLOT void on_btnNext_clicked();
		Q_SLOT void on_btnPrev_clicked();

		Q_SLOT void on_radioButton_toggled(bool);


		Q_SIGNAL void closed();

		std::vector<Jmcpp::UserId>	_getUserIds();

		void setGroupNameEnable(bool enabled)
		{
			ui.frameName->setVisible(enabled);
		}

		void setAdvanceGroupEnable(bool enabled)
		{
			ui.btnNext->setVisible(enabled);
			ui.btnOK->setVisible(!enabled);
		}

	private:
		Ui::SelectMemberWidget ui;
		ClientObjectPtr _co;
	};



}


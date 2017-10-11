#pragma once

#include <QWidget>
#include <QDebug>
#include "ui_GroupInfoWidget.h"

#include "ClientObject.h"

#include "MemberModel.h"

namespace JChat {

	class GroupInfoWidget : public QWidget
	{
		Q_OBJECT

	public:
		GroupInfoWidget(ClientObjectPtr const& co, int64_t groupId, QWidget *parent = Q_NULLPTR);
		~GroupInfoWidget();

		MemberModel* memberModel()
		{
			return _memberModel;
		}


		Q_SLOT void close();


		Q_SLOT void on_checkBoxNoDis_toggled(bool checked);
		Q_SLOT void on_checkBoxShield_toggled(bool checked);

		Q_SLOT void on_btnQuitGroup_clicked();

		Q_SLOT void on_btnAddMember_clicked();

		Q_SLOT void removeMember(Jmcpp::UserIdList const&);

	protected:

		None updateInfo();


		virtual bool eventFilter(QObject *watched, QEvent *event) override;

		virtual void paintEvent(QPaintEvent *event) override;

		virtual void showEvent(QShowEvent *event) override;

	private:
		Ui::GroupInfoWidget ui;

		MemberModel*	_memberModel = nullptr;

		bool active = false;

		ClientObjectPtr _co;
		int64_t			_groupId;
	};

} // namespace JChat

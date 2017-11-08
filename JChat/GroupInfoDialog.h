#pragma once

#include <QDialog>
#include "ui_GroupInfoDialog.h"

#include "ClientObject.h"
namespace JChat {

	class GroupInfoDialog : public QDialog
	{
		Q_OBJECT
	public:
		GroupInfoDialog(ClientObjectPtr const& co, Jmcpp::GroupId groupId, QWidget *parent = Q_NULLPTR);
		~GroupInfoDialog();

		pplx::task<void> setGroup(Jmcpp::GroupId groupId);



	protected:

		Q_SLOT void on_btnJoinGroup_clicked();

	private:
		Ui::GroupInfoDialog ui;

		ClientObjectPtr	_co;
		Jmcpp::GroupId	_groupId;
	};

} // namespace JChat



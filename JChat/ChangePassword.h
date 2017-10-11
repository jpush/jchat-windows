#pragma once

#include <QWidget>
#include "ui_ChangePassword.h"
#include "ClientObject.h"

namespace JChat {

class ChangePassword : public QWidget
{
	Q_OBJECT

public:
	ChangePassword(ClientObjectPtr const& co,  QWidget *parent = Q_NULLPTR);
	~ChangePassword();



	Q_SLOT void on_btnOK_clicked();
	Q_SLOT void on_btnCancel_clicked();

private:
	Ui::ChangePassword ui;

	ClientObjectPtr _co;

};

} // namespace JChat

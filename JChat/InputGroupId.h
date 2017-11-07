#pragma once

#include <QDialog>
#include "ui_InputGroupId.h"
#include "ClientObject.h"
namespace JChat {

	class InputGroupId : public QDialog
	{
		Q_OBJECT

	public:
		InputGroupId(ClientObjectPtr const& co,  QWidget *parent = Q_NULLPTR);
		~InputGroupId();



		Q_SLOT void on_btnOK_clicked();

		Q_SLOT void on_btnCancel_clicked()
		{
			close();
		}

	private:
		Ui::InputGroupId ui;

		ClientObjectPtr _co;

	};

} // namespace JChat

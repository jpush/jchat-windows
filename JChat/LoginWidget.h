#pragma once

#include <memory>
#include <QWidget>
#include "ui_LoginWidget.h"

#include "ClientObject.h"

namespace JChat{

	class LoginWidget : public QWidget
	{
		Q_OBJECT

	public:
		LoginWidget(QWidget *parent = Q_NULLPTR);
		~LoginWidget();


	private:

		void updateLoginButtonState()
		{
			if(ui.username->text().isEmpty() || ui.password->text().isEmpty())
			{
				ui.btnLogin->setEnabled(false);
			}
			else
			{
				ui.btnLogin->setEnabled(true);
			}
		}

		void updateRegisterButtonState()
		{
			if(ui.usernameR->text().isEmpty() || ui.password1->text().isEmpty() || ui.password2->text().isEmpty())
			{
				ui.btnRegister->setEnabled(false);
			}
			else
			{
				ui.btnRegister->setEnabled(true);
			}
		}

	protected:
		virtual void closeEvent(QCloseEvent *event) override;

	private:
		Ui::LoginWidget ui;

		std::shared_ptr<JChat::ClientObject> _co;
	};
}

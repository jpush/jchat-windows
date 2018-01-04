
#include "LoginWidget.h"
#include <QMessageBox>
#include <QxOrm.h>

#include "MainWidget.h"
#include "Emoji.h"
#include "BusyIndicator.h"
#include "RememberedAccount.h"


namespace JChat{

	LoginWidget::LoginWidget(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);
		setAttribute(Qt::WA_DeleteOnClose);

		this->setWindowFlags(Qt::WindowType::CustomizeWindowHint | Qt::WindowType::WindowCloseButtonHint);

		auto userIcon = QIcon(u8":/image/resource/用户名 icon.png");
		auto passwordIcon = QIcon(u8":/image/resource/密码 icon.png");

		ui.username->lineEdit()->addAction(userIcon, QLineEdit::LeadingPosition);
		ui.password->addAction(passwordIcon, QLineEdit::LeadingPosition);
		ui.btnLogin->setEnabled(false);

		ui.usernameR->addAction(userIcon, QLineEdit::LeadingPosition);
		ui.password1->addAction(passwordIcon, QLineEdit::LeadingPosition);
		ui.password2->addAction(passwordIcon, QLineEdit::LeadingPosition);
		ui.btnRegister->setEnabled(false);

		connect(ui.username->lineEdit(), &QLineEdit::returnPressed, ui.btnLogin, &QPushButton::click);
		connect(ui.password, &QLineEdit::returnPressed, ui.btnLogin, &QPushButton::click);


		connect(ui.usernameR, &QLineEdit::returnPressed, ui.btnRegister, &QPushButton::click);
		connect(ui.password1, &QLineEdit::returnPressed, ui.btnRegister, &QPushButton::click);
		connect(ui.password2, &QLineEdit::returnPressed, ui.btnRegister, &QPushButton::click);

		RememberedAccount data;
		auto accounts = data.getRememberedUsers();

		for(auto&& account : accounts)
		{
			if(!account.username.isEmpty() && !account.password.isEmpty())
			{
				ui.username->addItem(account.username);
			}
		}

		if(!accounts.empty())
		{
			ui.username->setCurrentIndex(0);
			auto&& account = accounts[0];
			if(!account.username.isEmpty() && !account.password.isEmpty())
			{
				ui.checkBox->setCheckState(Qt::Checked);
				ui.username->setEditText(account.username);
				ui.password->setText(account.password);
				ui.password->setProperty("encoded", true);
			}
		}

		connect(ui.username, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, [=](int i)
		{
			if(i == -1)
			{
				return;
			}
			auto&& account = accounts[i];
			if(!account.username.isEmpty() && !account.password.isEmpty())
			{
				ui.checkBox->setCheckState(Qt::Checked);
				ui.username->setEditText(account.username);
				ui.password->setText(account.password);
				ui.password->setProperty("encoded", true);
			}
		});


		updateLoginButtonState();

		connect(ui.username, &QComboBox::currentTextChanged, [=](QString const& str){updateLoginButtonState(); });
		connect(ui.password, &QLineEdit::textChanged, [=](QString const& str){	updateLoginButtonState(); ui.password->setProperty("encoded", false);   });

		connect(ui.usernameR, &QLineEdit::textChanged, [=](QString const& str){	updateRegisterButtonState(); });
		connect(ui.password1, &QLineEdit::textChanged, [=](QString const& str){	updateRegisterButtonState(); });
		connect(ui.password2, &QLineEdit::textChanged, [=](QString const& str){	updateRegisterButtonState(); });


		_co = std::make_shared<JChat::ClientObject>(ClientObject::getSDKConfig());

		connect(ui.btnLogin, &QPushButton::clicked, this, [=]() mutable
		{

			try
			{
				auto lock = std::make_unique<QSharedMemory>(ui.username->currentText());
				if(!lock->create(1))
				{
					lock->attach();//for unix
					lock->detach();//...
					if(!lock->create(1))
					{
						ui.labelError->setText(QString(u8"用户[%1]已登录,不能重复登录").arg(ui.username->currentText()));
						return;
					}
				}

				ui.labelError->clear();

				BusyIndicator busy(this);
				ui.btnLogin->setEnabled(false);

				auto mwPtr = std::make_unique<MainWidget>(_co);

				qAwait(pplx::create_task([]{
					Emoji::getSingleton()->moveToThread(qApp->thread());
				}));

				qAwait(_co->login(ui.username->currentText().toStdString(),
								  ui.password->text().toStdString(), ClientObject::getAuthorization(),
								  ui.password->property("encoded").toBool()));

				lock.release();

				Account accout;
				accout.username = ui.username->currentText();
				accout.password = ui.password->property("encoded").toBool() ? ui.password->text() : QCryptographicHash::hash(ui.password->text().toUtf8(), QCryptographicHash::Md5).toHex();

				if(ui.checkBox->isChecked())
				{
					data.addRememberedUsers(accout);
				}
				else
				{
					data.removeRememberedUsers(accout);
				}
				mwPtr->show();
				mwPtr.release();

				this->close();

				return;
			}
			catch(Jmcpp::ServerException& e)
			{
				if(e.code() == 880104 || e.code() == 880103)
				{
					ui.labelError->setText(u8"用户名或密码错误!");
				}
				else
				{
					ui.labelError->setText(e.what());
				}
			}
			catch(std::runtime_error& e)
			{
				ui.labelError->setText(e.what());
			}

			ui.btnLogin->setEnabled(true);
		});


		connect(ui.switchLogin, &QPushButton::clicked, this, [=]
		{
			ui.stackedWidget->setCurrentWidget(ui.pageLogin);
		});


		connect(ui.btnRegister, &QPushButton::clicked, this, [=]
		{
			try
			{
				if(ui.password1->text() != ui.password2->text())
				{
					ui.labelError2->setText(u8"您两次输入的密码不一致!");
					return;
				}
				ui.labelError2->clear();
				ui.btnRegister->setEnabled(false);
				qAwait(_co->registers(ui.usernameR->text().toStdString(), ui.password1->text().toStdString(), ClientObject::getAuthorization()));
				QMessageBox::information(this, tr("info"), u8"注册成功!");
			}
			catch(Jmcpp::ServerException& e)
			{
				if(e.code() == 882002)
				{
					ui.labelError2->setText(u8"用户名已存在!");
				}
				else
				{
					ui.labelError2->setText(e.what());
				}
			}
			catch(std::system_error& e)
			{
				ui.labelError2->setText(e.what());
			}

			ui.btnRegister->setEnabled(true);
		});

		connect(ui.switchRegister, &QPushButton::clicked, this, [=]
		{
			ui.stackedWidget->setCurrentWidget(ui.pageRegister);
		});
	}

	LoginWidget::~LoginWidget()
	{
		_co.reset();
	}

	void LoginWidget::closeEvent(QCloseEvent *event)
	{
		if(!_co->isLogined())
		{
			qApp->quit();
		}
	}
}

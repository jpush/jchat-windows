#pragma once

#include <QWidget>
#include "ui_UserInfoWidget.h"

#include "ClientObject.h"

namespace JChat
{
	class UserInfoWidget : public QWidget
	{
		Q_OBJECT

	public:
		enum Mode
		{
			self,
			friends,
			stranger
		};

		UserInfoWidget(ClientObjectPtr const& co, QWidget *parent = Q_NULLPTR);

		~UserInfoWidget();

		void setEditable(bool editable);


		void setMode(Mode m);


		void setAvatar(QPixmap const& pixmap)
		{
			ui.label->setPixmap(pixmap);
		}

		void setUserInfo(Jmcpp::UserInfo const& userInfo);


		pplx::task<void> updateSelfInfo();


		static void showUserInfo(ClientObjectPtr const& _co, Jmcpp::UserId const& userId, QWidget* parent = nullptr);

		static void showSelfInfo(ClientObjectPtr const& co, QWidget* parent = nullptr);

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::UserInfoWidget ui;


		ClientObjectPtr _co;

		Jmcpp::UserInfo _userInfo;
		Jmcpp::UserId	_userId;

		Mode _mode;
		bool _editable = false;

	};

}
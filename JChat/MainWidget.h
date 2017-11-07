#pragma once

#include <QDebug>
#include <QMenu>
#include <QWidget>
#include "ui_MainWidget.h"

#include <Jmcpp/Client.h>

#include "await.h"
#include "ChatWidget.h"
#include "MetaTypes.h"

#include "ClientObject.h"
#include "ConversationModel.h"
#include "ProxyModel.h"

class QSystemTrayIcon;

namespace JChat{
	class TrayIconMessage;

	class MainWidget : public QWidget
	{
		Q_OBJECT
	public:

		MainWidget(JChat::ClientObjectPtr const& co, QWidget *parent = Q_NULLPTR);

		~MainWidget();


		void			switchToConversation(Jmcpp::ConversationId const& conId);

		ChatWidget*		getOrCreateChatWidget(Jmcpp::ConversationId const& conId);
		ChatWidget*		getChatWidget(Jmcpp::ConversationId const& conId);

		ConversationModel*	getConversationModel() const{ return _conModel; }
		ProxyModel*			getConversationProxyModel() const;


		static MainWidget* getCurrentMainWidget()
		{
			return qApp->property(MainWidget::staticMetaObject.className()).value<MainWidget*>();
		}
	protected:

		void flashTrayIcon(bool start);


		void initTrayIcon();

		void initMenu();

		void initMessagePage();
		void initContactPage();

		void initEvent();


		void			_switchToConversation(Jmcpp::ConversationId const& conId);

		//////////////////////////////////////////////////////////////////////////

		None			onSearch();

		void			searchUser(Jmcpp::UserId const& userId);

		void			searchGroup(Jmcpp::GroupId groupId);


		void			createGroup();


		void			showBlackList();

		void			logout();



		None	updateSelfInfo(bool update = false)
		{
			auto self = this | qTrack;
			auto info = co_await _co->getCacheUserInfo(_co->getCurrentUser(), update);
			auto pixmap = co_await _co->getCacheUserAvatar(_co->getCurrentUser(), info.avatar, update);
			co_await self;
			ui.labelAvatar->setPixmap(pixmap);
		}

		//template<class T>
		//void trackTask(pplx::task<T> t)
		//{
		//	auto tsk = t.then([](pplx::task<T> t){
		//		try
		//		{
		//			t.get();
		//		}
		//		catch(...)
		//		{
		//		}
		//	});
		//	_tasks.emplace_back(std::move(tsk));
		//}
		//
		//
		//void cleanReadyTask()
		//{
		//	auto iter = std::remove_if(_tasks.begin(), _tasks.end(), [](pplx::task<void> const&t)
		//	{
		//		return t.is_done();
		//	});
		//	_tasks.erase(iter, _tasks.end());
		//}
		//
		//void awaitAllTask()
		//{
		//	auto t = pplx::when_all(_tasks.begin(), _tasks.end());
		//	try
		//	{
		//		qAwait(t);
		//	}
		//	catch(std::runtime_error& e)
		//	{
		//
		//	}
		//}


		virtual bool eventFilter(QObject *watched, QEvent *event) override;
		virtual void closeEvent(QCloseEvent *event) override;
		virtual void showEvent(QShowEvent *event) override;

	private:
		Ui::MainWidgetClass ui;

		std::map<Jmcpp::ConversationId, QPointer< ChatWidget>>	_chatWidgets;

		JChat::ClientObjectPtr		_co;
		TrayIconMessage*			_trayMessage;
		QSystemTrayIcon*			_tray;
		QTimer*						_trayIconTimer = new QTimer(this);

		ConversationModel*			_conModel;
		ProxyModel*					_conPrxModel;

		std::map<Jmcpp::ConversationId, Jmcpp::UserInfo> _userInfos;
		std::map<Jmcpp::ConversationId, Jmcpp::GroupInfo> _groupInfos;


		std::vector<pplx::task<void> > _tasks;
	};
}

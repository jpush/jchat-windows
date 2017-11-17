
#include "MainWidget.h"

#include <QStringBuilder>
#include <QDateTime>
#include <QFileDialog>
#include <QPointer>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMouseEvent>
#include <QMessageBox>
#include <QProcess>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonValue>

#include <QxModelView.h>

#include "Dispatch.h"
#include "MetaTypes.h"

#include "ConversationModel.h"
#include "ContactModel.h"

#include "UserInfoWidget.h"
#include "BusyIndicator.h"

#include "EnterUserNameWidget.h"
#include "ItemWidgetMapper.h"

#include "ConversationItemWidget.h"
#include "SelectMemberWidget.h"
#include "ChangePassword.h"

#include "TrayIconMessage.h"

#include "GroupInfoDialog.h"

#include "InputGroupId.h"

#include "LoginWidget.h"
#include "BlackList.h"
#include "ModelRange.h"

#include "RoomListModel.h"
#include "RoomItemWidget.h"

using namespace JChat;

MainWidget::MainWidget(JChat::ClientObjectPtr const& co, QWidget *parent /*= Q_NULLPTR*/)
	: QWidget(parent)
	, _co(co)
{
	qApp->setProperty(staticMetaObject.className(), QVariant::fromValue(this));
	ui.setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose);

	ui.listWidgetFriendEvent->setClientObject(_co);

	ui.listWidgetGroupEvent->setClientObject(_co);

	initTrayIcon();

	connect(ui.labelAvatar, &Label::clicked, this, [=]
	{
		UserInfoWidget::showSelfInfo(_co, this);
	});

	auto search = ui.lineEditSearch->addAction(QIcon(u8":/image/resource/搜索.png"), QLineEdit::TrailingPosition);
	connect(search, &QAction::triggered, this, &MainWidget::onSearch);

	connect(ui.lineEditSearch, &QLineEdit::returnPressed, this, &MainWidget::onSearch);
	connect(ui.lineEditSearch, &QLineEdit::textChanged, this, [=](QString const& text)
	{
		if(text.isEmpty())
		{
			onSearch();
		}
	});

	auto btnGroup = new QButtonGroup(this);
	btnGroup->addButton(ui.btnMessages);
	btnGroup->addButton(ui.btnContacts);
	btnGroup->addButton(ui.btnRooms);


	ui.pageMessages->installEventFilter(this);
	connect(ui.btnMessages, &QToolButton::clicked, this, [=]	{
		ui.stackedWidget->setCurrentWidget(ui.pageMessages);
	});

	connect(ui.btnContacts, &QToolButton::clicked, this, [=]	{
		ui.stackedWidget->setCurrentWidget(ui.pageContacts);
	});

	connect(ui.btnRooms, &QToolButton::clicked, this, [=]	{

		auto roomId = _co->getCurrentRoomId();
		if(roomId.get())
		{
			if(auto w = getChatWidget(roomId))
			{
				ui.stackedWidgetRoom->setCurrentWidget(w);
			}
		}

		ui.stackedWidget->setCurrentWidget(ui.pageRooms);
	});


	initMenu();
	initMessagePage();
	initContactPage();
	initRoomPage();
	initEvent();

}


MainWidget::~MainWidget()
{
	if(_co->isLogined())
	{
		_co->logout();
	}
}



void
JChat::MainWidget::switchToConversation(Jmcpp::ConversationId const& conId)
{
	if(conId.isRoom())
	{
		ui.btnRooms->click();
		auto w = getOrCreateChatWidget(conId);
		ui.stackedWidgetRoom->setCurrentWidget(w);
		return;
	}

	auto item = _conModel->addConversationItem(conId);
	ui.btnMessages->click();

	if(ui.listConversation->currentIndex() == _conPrxModel->mapFromSource(item->index()))
	{
		_switchToConversation(conId);
	}
	else
	{
		ui.listConversation->setCurrentIndex(_conPrxModel->mapFromSource(item->index()));
	}
}


void
JChat::MainWidget::_switchToConversation(Jmcpp::ConversationId const& conId)
{
	if(conId.isRoom())
	{
		return;
	}

	auto w = getOrCreateChatWidget(conId);
	ui.stackedWidgetChat->setCurrentWidget(w);

	auto tmp = w->listWidget()->getUnreadMessageCount();
	w->listWidget()->setUnreadMessageCount(0);
	_conModel->setUnreadMessageCountRole(conId, 0);

	if(tmp)
	{
		_co->clearUnreadCount(conId);
	}
}



void
JChat::MainWidget::searchUser(Jmcpp::UserId const& userId)
{
	UserInfoWidget::showUserInfo(_co, userId, this);
}

void
JChat::MainWidget::searchGroup(Jmcpp::GroupId groupId)
{

}

None
JChat::MainWidget::onSearch()
{
	if(ui.lineEditSearch->text().isEmpty())
	{
		_conPrxModel->invalidateFilter();
		return;
	}

	std::vector<Jmcpp::ConversationId> conIds;
	for(auto&& item : _conModel | depthFirst)
	{
		auto conId = item->data(ConversationModel::ConIdRole).value<Jmcpp::ConversationId>();
		conIds.emplace_back(conId);
	}

	auto co = _co;
	auto self = this | qTrack;

	std::map<Jmcpp::ConversationId, Jmcpp::UserInfo> userInfos;
	std::map<Jmcpp::ConversationId, Jmcpp::GroupInfo> groupInfos;

	for(auto&& con : conIds)
	{
		if(con.isUser())
		{
			auto info = co_await co->getCacheUserInfo(con.getUserId());
			userInfos.insert_or_assign(con, info);
		}
		else
		{
			auto info = co_await co->getCacheGroupInfo(con.getGroupId());
			groupInfos.insert_or_assign(con, info);
		}
	}

	co_await self;

	_userInfos = userInfos;
	_groupInfos = groupInfos;
	_conPrxModel->invalidateFilter();
}

void
JChat::MainWidget::createGroup()
{
	auto self = this | qTrack;
	auto co = _co;
	QString groupName;
	QString groupAvatar;
	bool isPublic = false;
	auto userIds = SelectMemberWidget::getUserIds(_co, u8"发起群聊", groupName, groupAvatar, isPublic, this);
	if(!userIds)
	{
		return;
	}

	BusyIndicator busy(this);
	Jmcpp::ConversationId conId;
	try
	{
		auto info = qAwait(co->createGroup(groupName.toStdString(), {}, groupAvatar.toStdString(), isPublic));
		conId = info.groupId;
		qAwait(co->addGroupMembers(info.groupId, *userIds));
	}
	catch(std::runtime_error& e)
	{
		QMessageBox::warning(this, "", e.what(), QMessageBox::Ok);
	}

	if(conId.isGroup())
	{
		switchToConversation(conId);
	}
}

void
JChat::MainWidget::showBlackList()
{
	BusyIndicator busy(this);
	auto widget = new BlackList(_co, this);
	qAwait(widget->getBlackList());
	widget->show();
}

void
JChat::MainWidget::logout()
{
	static auto exe = QCoreApplication::applicationFilePath();
	qAddPostRoutine([]{QProcess::startDetached(exe); });
	qApp->quit();
}



JChat::ChatWidget*
JChat::MainWidget::getOrCreateChatWidget(Jmcpp::ConversationId const& conId)
{
	auto iter = _chatWidgets.find(conId);
	if(iter != _chatWidgets.end() && iter->second)
	{
		return iter->second;
	}

	if(conId.isRoom())
	{
		auto w = new ChatWidget(_co, conId, ui.stackedWidgetRoom);
		_chatWidgets.insert_or_assign(conId, w);
		ui.stackedWidgetRoom->addWidget(w);
		return w;
	}
	else
	{
		auto w = new ChatWidget(_co, conId, ui.stackedWidgetChat);
		_chatWidgets.insert_or_assign(conId, w);
		ui.stackedWidgetChat->addWidget(w);
		return w;
	}
}

JChat::ChatWidget*
JChat::MainWidget::getChatWidget(Jmcpp::ConversationId const& conId)
{
	auto iter = _chatWidgets.find(conId);
	if(iter != _chatWidgets.end())
	{
		return iter->second;
	}
	return nullptr;
}

void
JChat::MainWidget::removeChatWidget(Jmcpp::ConversationId const& conId)
{
	auto iter = _chatWidgets.find(conId);
	if(iter != _chatWidgets.end())
	{
		delete iter->second;
		_chatWidgets.erase(iter);
	}
}


JChat::ProxyModel*
JChat::MainWidget::getConversationProxyModel() const
{
	return _conPrxModel;
}


void
JChat::MainWidget::switchToRoomPage(Jmcpp::RoomId const& roomId)
{
	ui.pageRoomInfo->setRoomId(_co, roomId);
	ui.stackedWidgetRoom->setCurrentWidget(ui.pageRoomInfo);
}

//////////////////////////////////////////////////////////////////////////

bool
JChat::MainWidget::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == ui.pageMessages && event->type() == QEvent::Show)
	{
		if(ui.listConversation->currentIndex().isValid())
		{
			auto conId = ui.listConversation->currentIndex().data(ConversationModel::Role::ConIdRole).value<Jmcpp::ConversationId>();
			_switchToConversation(conId);
		}
	}
	return false;
}

void
JChat::MainWidget::showEvent(QShowEvent *event)
{
	flashTrayIcon(false);
	if(ui.stackedWidget->currentWidget() == ui.pageMessages)
	{
		if(ui.listConversation->currentIndex().isValid())
		{
			auto conId = ui.listConversation->currentIndex().data(ConversationModel::Role::ConIdRole).value<Jmcpp::ConversationId>();
			_switchToConversation(conId);
		}
	}
}

void
MainWidget::closeEvent(QCloseEvent *event)
{
	event->ignore();
	hide();
	static bool firstTime = true;
	if(firstTime)
	{
		_tray->showMessage("JChat", u8"JChat已最小化", QSystemTrayIcon::Information, 1000);
		firstTime = false;
	}
}



//////////////////////////////////////////////////////////////////////////

Q_GLOBAL_STATIC_WITH_ARGS(QIcon, trayIncon, (":/image/resource/icon.png"))

void
JChat::MainWidget::flashTrayIcon(bool start)
{
	if(start)
	{
		_trayIconTimer->start(500);
		_trayMessage->show();
	}
	else
	{
		_trayIconTimer->stop();
		_tray->setIcon(*trayIncon);
		_trayMessage->hide();
	}
}

void
JChat::MainWidget::initTrayIcon()
{
	_tray = new QSystemTrayIcon(this);
	_tray->setIcon(*trayIncon);
	_tray->show();
	auto trayMenu = new QMenu(this);
	trayMenu->addAction(u8"打开JChat", [=]{this->showNormal(); });
	trayMenu->addAction(u8"退出", []{QCoreApplication::exit(); });
	_tray->setContextMenu(trayMenu);
	connect(_tray, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason)
	{
		if(reason == QSystemTrayIcon::Trigger)
		{
			this->activateWindow();
			this->showNormal();
		}
	});
	connect(_trayIconTimer, &QTimer::timeout, this, [=]
	{
		if(!_trayIconTimer->isActive())
		{
			_tray->setIcon(*trayIncon);
		}
		if(_tray->icon().isNull())
		{
			_tray->setIcon(*trayIncon);
		}
		else
		{
			_tray->setIcon({});
		}
	});
}

void
JChat::MainWidget::initMenu()
{
	auto addMenu = new QMenu(this);
	addMenu->addAction(u8"发起单聊", [=]
	{
		auto name = EnterUserNameWidget::getUserName(this);
		if(name.isEmpty())
			return;
		searchUser(Jmcpp::UserId(name.toStdString()));
	});

	addMenu->addAction(u8"发起群聊", this, &MainWidget::createGroup);
	addMenu->addAction(u8"添加好友", [=]
	{
		auto name = EnterUserNameWidget::getUserName(this, u8"添加好友");
		if(name.isEmpty())
			return;
		searchUser(Jmcpp::UserId(name.toStdString()));
	});

	addMenu->addAction(u8"加入公开群", this, [=]
	{
		InputGroupId* w = new InputGroupId(_co, this);
		w->setAttribute(Qt::WA_DeleteOnClose);
		w->setModal(true);
		w->show();
	});

	connect(ui.btnAdd, &QToolButton::clicked, this, [=]{
		addMenu->popup(QCursor::pos());
	});

	//
	auto moreMenu = new QMenu(this);
	moreMenu->addAction(u8"修改密码", this, [=]{
		(new ChangePassword(_co, this))->show();
	});

	moreMenu->addAction(u8"黑名单列表", this, &MainWidget::showBlackList);
	moreMenu->addAction(u8"退出登录", this, &MainWidget::logout);

	connect(ui.btnMenu, &QToolButton::clicked, this, [=]{
		moreMenu->popup(QCursor::pos());
	});
}


void
JChat::MainWidget::initMessagePage()
{
	_conModel = new ConversationModel(_co.get(), this);
	_conPrxModel = new ProxyModel(this);
	_conPrxModel->sort(0);
	_conPrxModel->setSourceModel(_conModel);
	_conPrxModel->setDataFnForRole(Qt::SizeHintRole, [](QModelIndex const& index, QVariant& data)
	{
		auto sz = data.toSize();
		sz.setHeight(70);
		data = sz;
	});

	_conPrxModel->setFilterFn([=](int source_row, const QModelIndex &source_parent, bool& ret)
	{
		auto name = ui.lineEditSearch->text().toLower();
		if(name.isEmpty())
		{
			return;
		}

		auto index = _conModel->index(source_row, 0, source_parent);
		auto item = _conModel->itemFromIndex(index);

		auto conId = item->data(ConversationModel::ConIdRole).value<Jmcpp::ConversationId>();

		if(auto iter = _userInfos.find(conId); iter != _userInfos.end())
		{
			auto info = iter->second;
			auto username = QString::fromStdString(info.userId.username).toLower();
			auto nickname = QString::fromStdString(info.nickname).toLower();
			auto remark = QString::fromStdString(info.remark).toLower();

			if(remark.contains(name) || nickname.contains(name) || username.contains(name))
			{
				ret = true;
			}
			else
			{
				ret = false;
			}
		}
		else if(auto iter = _groupInfos.find(conId); iter != _groupInfos.end())
		{
			auto info = iter->second;
			auto groupName = QString::fromStdString(info.groupName).toLower();
			if(groupName.contains(name))
			{
				ret = true;
			}
			else
			{
				ret = false;
			}
		}
	});

	_conPrxModel->setCompareFn(ConversationModel::sortFn);

	ui.listConversation->setModel(_conPrxModel);

	auto mapper = new ItemWidgetMapper(this);
	mapper->setView(ui.listConversation);
	mapper->setItemWidgetClass<ConversationItemWidget>();

	mapper->addMapping(ConversationModel::ImageRole, "avatar");
	mapper->addMapping(ConversationModel::MessageRole, "message");
	mapper->addMapping(ConversationModel::TitleRole, "title");
	mapper->addMapping(ConversationModel::TimeRole, "dateTime");
	mapper->addMapping(ConversationModel::ShieldRole, "isShield");
	mapper->addMapping(ConversationModel::NotDisturb, "isNotDisturb");
	mapper->addMapping(ConversationModel::UnreadRole, "unreadMessageCount");
	mapper->addMapping(ConversationModel::SticktopTimeRole, [](QWidget* widget, QVariant const& value)
	{
		auto w = (ConversationItemWidget*)widget;
		w->setSticktop(!value.toDateTime().isNull());
	});

	connect(mapper, &ItemWidgetMapper::itemWidgetCreated, this, [=](QWidget* widget, QModelIndex const& index)
	{
		auto w = static_cast<ConversationItemWidget*>(widget);
		auto idx = QPersistentModelIndex(_conPrxModel->mapToSource(index));
		connect(w, &ConversationItemWidget::closeClicked, this, [=]
		{
			_conModel->removeRow(idx.row());
		});

		connect(w, &ConversationItemWidget::sticktopChanged, this, [=](bool checked)
		{
			auto item = _conModel->itemFromIndex(idx);
			auto conId = item->data(ConversationModel::ConIdRole).value<Jmcpp::ConversationId>();
			if(checked)
			{
				item->setData(QDateTime::currentDateTime(), ConversationModel::SticktopTimeRole);
			}
			else
			{
				item->setData(QDateTime(), ConversationModel::SticktopTimeRole);
			}
			_conModel->setSticktop(conId, checked);
		});
	});

	connect(ui.listConversation->selectionModel(), &QItemSelectionModel::currentChanged,
			this, [=](const QModelIndex &current, const QModelIndex &previous)
	{
		if(previous.isValid())
		{
			auto conId = previous.data(ConversationModel::Role::ConIdRole).value<Jmcpp::ConversationId>();
			auto w = getChatWidget(conId);
			if(w)
			{
				auto text = w->getEditingText();
				if(!text.isEmpty())
				{
					QString richText = u8R"(<span style=" color:#ff0000;">[草稿]</span><span>%1</span>)";
					_conModel->setMessageRole(conId, richText.arg(text));
				}
			}
		}

		if(current.isValid()){
			auto conId = current.data(ConversationModel::Role::ConIdRole).value<Jmcpp::ConversationId>();
			_switchToConversation(conId);
			_conModel->updateMessageRole(conId);
		}
		else
		{
			ui.stackedWidgetChat->setCurrentWidget(ui.page);
		}
	});

	connect(_conPrxModel, &QAbstractItemModel::rowsAboutToBeRemoved,
			this, [=](const QModelIndex &parent, int first, int last)
	{
		for(; first <= last; ++first)
		{
			auto index = _conPrxModel->index(first, 0, parent);
			auto conId = index.data(ConversationModel::Role::ConIdRole).value<Jmcpp::ConversationId>();
			removeChatWidget(conId);
		}
	});


	connect(_conModel, &ConversationModel::unreadMessageCountChanged, this, [=]
	{
		ui.btnMessages->setCount(_conModel->getTotalUnreadMessageCount());
	});

	//////////////////////////////////////////////////////////////////////////
	_trayMessage = new TrayIconMessage(_tray, this);
	connect(_trayMessage, &TrayIconMessage::cancelFlash, this, [=]
	{
		flashTrayIcon(false);
	});
	connect(_trayMessage, &TrayIconMessage::showMessage, this, [=](Jmcpp::ConversationId const& conId)
	{
		showNormal();
		this->activateWindow();
		switchToConversation(conId);
	});
}


//////////////////////////////////////////////////////////////////////////
void
JChat::MainWidget::initContactPage()
{
	auto contactModel = new ContactModel(_co, this);
	ui.listContactType->setModel(contactModel);
	ui.listContactType->setItemDelegate(new UnreadCountDelegate(this));

	auto delegate = new ContactDelegate();
	ui.listViewFriend->setModel(contactModel);
	ui.listViewFriend->setRootIndex(contactModel->getFriendRootItem()->index());
	ui.listViewFriend->setItemDelegate(delegate);

	ui.listViewGroup->setModel(contactModel);
	ui.listViewGroup->setRootIndex(contactModel->getGroupRootItem()->index());
	ui.listViewGroup->setItemDelegate(delegate);

	connect(ui.listViewFriend, &QListView::clicked, this, [=](const QModelIndex &index){
		auto info = index.data(ContactModel::InfoRole).value<Jmcpp::UserInfo>();
		switchToConversation(info.userId);
	});


	connect(ui.listViewGroup, &QListView::clicked, this, [=](const QModelIndex &index){
		auto info = index.data(ContactModel::InfoRole).value<Jmcpp::GroupInfo>();
		switchToConversation(info.groupId);
	});

	connect(ui.listContactType->selectionModel(), &QItemSelectionModel::currentChanged,
			this, [=](const QModelIndex &current, const QModelIndex &previous)
	{
		if(current.isValid())
		{
			ui.stackedWidgetContact->setCurrentIndex(current.row());
			if(current.row() == 0)
			{
				if(ui.tabWidget->currentIndex() == 0)
				{
					ui.listWidgetFriendEvent->clearUnread();
				}
				else
				{
					ui.listWidgetGroupEvent->clearUnread();
				}
			}
		}
	});


	connect(ui.tabWidget, &QTabWidget::currentChanged,
			this, [=](int index)
	{
		if(index == 0)
		{
			ui.listWidgetFriendEvent->clearUnread();
		}
		else
		{
			ui.listWidgetGroupEvent->clearUnread();
		}
	});


	connect(ui.listWidgetFriendEvent, &FriendEventListWidget::unreadChanged, this, [=](int count)
	{
		auto total = count + ui.listWidgetGroupEvent->unreadCount();
		contactModel->getFriendEventRootItem()->setData(total);
		ui.btnContacts->setCount(total);

		ui.tabWidget->setTabText(0, u8"好友验证" + (count ? QString("(%1)").arg(count) : ""));
	});

	connect(ui.listWidgetGroupEvent, &GroupEventWidget::unreadChanged, this, [=](int count)
	{
		auto total = count + ui.listWidgetFriendEvent->unreadCount();
		contactModel->getFriendEventRootItem()->setData(total);
		ui.btnContacts->setCount(total);

		ui.tabWidget->setTabText(0, u8"群组验证" + (count ? QString("(%1)").arg(count) : ""));

	});

	ui.listContactType->setCurrentIndex(contactModel->getFriendRootItem()->index());
}

void
JChat::MainWidget::initRoomPage()
{

	auto roomModel = new RoomListModel(_co, this);

	ui.listRoom->setModel(roomModel);

	auto mapper = new ItemWidgetMapper(this);

	mapper->setItemWidgetClass<RoomItemWidget>();
	mapper->addMapping(RoomListModel::Role::NameRole, "title");
	mapper->addMapping(RoomListModel::Role::DescRole, "desc");
	mapper->addMapping(RoomListModel::Role::ImageRole, "avatar");

	mapper->setView(ui.listRoom);


	connect(ui.listRoom->selectionModel(), &QItemSelectionModel::currentChanged,
			this, [=](const QModelIndex &current, const QModelIndex &previous)
	{

		if(current.isValid()){
			auto roomId = current.data(RoomListModel::RoomIdRole).value<Jmcpp::RoomId>();

			auto w = getChatWidget(roomId);
			if(w)
			{
				ui.stackedWidgetRoom->setCurrentWidget(w);
			}
			else
			{
				ui.pageRoomInfo->setRoomId(_co, roomId);
				ui.stackedWidgetRoom->setCurrentWidget(ui.pageRoomInfo);
			}
		}
		else
		{
			ui.pageRoomInfo->setRoomId(_co, 0);
			ui.stackedWidgetRoom->setCurrentWidget(ui.pageWelcome);
		}
	});
}

//////////////////////////////////////////////////////////////////////////
void
JChat::MainWidget::initEvent()
{
	connect(_co.get(), &ClientObject::userLogined, this, [=]()
	{
		updateSelfInfo();
	});

	connect(_co.get(), &ClientObject::disconnected, this, [=]()
	{
		BusyIndicator busy(this);
		for(;;)
		{
			try
			{
				qAwait(_co->relogin(ClientObject::getAuthorization()));
				break;
			}
			catch(std::runtime_error& e)
			{
				qAwait(std::chrono::seconds(5));
			}
		}
	});


	connect(_co.get(), &ClientObject::selfInfoUpdated, this, [=]()
	{
		updateSelfInfo();
	});


	connect(_co.get(), &ClientObject::transCommandEvent, this, [=](Jmcpp::TransCommandEvent const& e)
	{
		auto conId = e.conId;
		auto w = getChatWidget(conId);
		if(w)
		{
			auto json = QJsonDocument::fromJson(QByteArray::fromStdString(e.cmd)).object().toVariantMap();
			auto type = json.value("type").toString();
			if(type == "input")
			{
				auto content = json.value("content").toMap();
				auto message = content.value("message").toString();
				w->onInputtingStatusChanged(!message.isEmpty());
			}
		}
	});

	connect(_co.get(), &ClientObject::messageReceived, this, [=](Jmcpp::MessagePtr const& msg)
	{
		QApplication::alert(this);
		auto conId = msg->conId;
		auto w = getOrCreateChatWidget(conId);

		w->listWidget()->insertMessage(msg);

		if(conId.isRoom())
		{
			return;
		}

		if(w->isVisible())
		{
			w->listWidget()->setUnreadMessageCount(0);
			_conModel->setUnreadMessageCountRole(conId, 0);
			_co->clearUnreadCount(conId);
		}
		else
		{
			if(!msg->isOutgoing)
			{
				auto count = w->listWidget()->getUnreadMessageCount() + 1;
				w->listWidget()->setUnreadMessageCount(count);
				_conModel->setUnreadMessageCountRole(conId, count);
			}
		}

		if(!msg->isOutgoing)
		{
			flashTrayIcon(!isVisible());

			if(ui.stackedWidget->currentWidget() != ui.pageMessages)
			{
				//ui.btnMessages->addCount(1);
			}
		}

	});

	connect(_co.get(), &ClientObject::messagesReceived, this, [=](std::vector<Jmcpp::MessagePtr> const& msgs)
	{
		QApplication::alert(this);
		auto conId = msgs.back()->conId;
		auto w = getOrCreateChatWidget(conId);

		if(conId.isRoom())
		{
			for(auto&& m : msgs)
			{
				w->listWidget()->insertMessage(m);
				qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
			}
			return;
		}
		else
		{
			if(w->listWidget()->getFirstLoadedMessage())
			{
				w->listWidget()->clear();
				w->loadMessage(15);
			}
		}


		auto sz = std::count_if(msgs.begin(), msgs.end(), [](Jmcpp::MessagePtr const& msg)
		{
			return !msg->isOutgoing;
		});

		if(w->isVisible())
		{
			w->listWidget()->setUnreadMessageCount(0);
			_conModel->setUnreadMessageCountRole(conId, 0);
			_co->clearUnreadCount(conId);
		}
		else
		{
			auto count = w->listWidget()->getUnreadMessageCount() + sz;
			w->listWidget()->setUnreadMessageCount(count);
			_conModel->setUnreadMessageCountRole(conId, count);
		}

		if(sz)
		{
			flashTrayIcon(!isVisible());

			if(ui.stackedWidget->currentWidget() != ui.pageMessages)
			{
				//ui.btnMessages->addCount(sz);
			}
		}

	});

	connect(_co.get(), &ClientObject::forceLogoutEvent, this, [=](Jmcpp::ForceLogoutEvent const& e){
		QString msg;
		switch(e.reason){
			case Jmcpp::ForceLogoutEvent::Logined:msg = u8"异地登陆,强制退出,请重新登陆"; break;
			case Jmcpp::ForceLogoutEvent::PasswordChanged:msg = u8"密码已修改,请重新登陆"; break;
			default:msg = u8"强制退出,请重新登陆"; break;
		}
		QMessageBox::warning(this, "", msg, QMessageBox::Ok);
		logout();
	});

	connect(_co.get(), &ClientObject::messageRetracted, this, [=](Jmcpp::MessageRetractedEvent const& e){
		auto w = getOrCreateChatWidget(e.conId);
		w->listWidget()->onRetractMessage(e);
	});

	connect(_co.get(), &ClientObject::multiUnreadMsgCountChangedEvent, this, [=](Jmcpp::MultiUnreadMsgCountChangedEvent const& e){
		auto w = getChatWidget(e.conId);
		if(w)
		{
			w->listWidget()->setUnreadMessageCount(e.unreadMsgCount);
			_conModel->setUnreadMessageCountRole(e.conId, e.unreadMsgCount);
		}
	});

	connect(_co.get(), &ClientObject::receiptsUpdatedEvent, this, [=](Jmcpp::ReceiptsUpdatedEvent const& e){
		auto w = getChatWidget(e.conId);
		if(w)
		{
			w->listWidget()->updateMessageReceipts(e);
		}
	});

	connect(_co.get(), &ClientObject::groupCreatedEvent, this,
			[=](Jmcpp::GroupCreatedEvent const& e)
	{

	});

	connect(_co.get(), &ClientObject::groupInfoUpdatedEvent, this,
			[=](Jmcpp::GroupInfoUpdatedEvent e)  ->None
	{
		auto co = _co;
		auto self = this | qTrack;
		auto w = getOrCreateChatWidget(e.groupId) | qTrack;

		QString text = e.fromUser == co->getCurrentUser() ? u8"我" : co_await co->getUserDisplayName(e.fromUser);
		text += u8"修改了群信息";
		co_await w;
		w->listWidget()->insertCenterWidget(text);

		_conModel->setMessageRole(e.groupId, text);
	});


	connect(_co.get(), &ClientObject::addedToGroupEvent, this,
			[=](Jmcpp::AddedToGroupEvent e) ->None
	{
		auto co = _co;
		auto self = this | qTrack;
		auto w = getOrCreateChatWidget(e.groupId) | qTrack;

		QString text;

		if(e.fromUser == co->getCurrentUser() && e.users.size() == 1 && e.users[0] == e.fromUser)
		{
			text = u8"我加入了群聊";
		}
		else
		{
			text = e.fromUser == co->getCurrentUser() ? u8"我" : co_await co->getUserDisplayName(e.fromUser);
			text += u8"邀请 ";
			QStringList users;
			for(auto&& userId : e.users)
			{
				users << co_await co->getUserDisplayName(userId);
				if(users.size() > 3)
				{
					break;
				}
			}

			text += users.join(',');
			if(e.users.size() > (size_t)users.size())
			{
				text += u8"...";
			}
			text += u8" 加入了群聊";
		}

		co_await w;
		w->listWidget()->insertCenterWidget(text);

		_conModel->setMessageRole(e.groupId, text);
	});

	connect(_co.get(), &ClientObject::leavedGroupEvent, this,
			[=](Jmcpp::LeavedGroupEvent e) ->None
	{
		auto co = _co;
		auto self = this | qTrack;
		if(_co->getCurrentUser() == e.fromUser)
		{
			_conModel->remove(e.groupId);
			co_return;
		}

		auto w = getOrCreateChatWidget(e.groupId) | qTrack;
		QString text = co_await co->getUserDisplayName(e.fromUser);
		text += u8" 退出了群聊";
		co_await w;
		w->listWidget()->insertCenterWidget(text);

		_conModel->setMessageRole(e.groupId, text);

	});

	connect(_co.get(), &ClientObject::removedFromGroupEvent, this,
			[=](Jmcpp::RemovedFromGroupEvent e) ->None
	{
		auto co = _co;
		auto self = this | qTrack;
		auto w = getOrCreateChatWidget(e.groupId) | qTrack;
		QString text = co_await co->getUserDisplayName(e.fromUser);
		text += u8"把 ";
		QStringList users;

		auto selfUserId = co->getCurrentUser();
		for(auto&& userId : e.users)
		{
			users << (userId == selfUserId ? u8"我" : co_await co->getUserDisplayName(userId));
		}

		text += users.join(',');
		text += u8" 移出了群聊";

		co_await w;
		w->listWidget()->insertCenterWidget(text);

		_conModel->setMessageRole(e.groupId, text);
	});


	connect(_co.get(), &ClientObject::groupMemberSilentChangedEvent, this, [=](Jmcpp::GroupMemberSilentChangedEvent const &e)
	{
		auto w = getOrCreateChatWidget(e.groupId) | qTrack;

		QString text = e.on ? QString(u8"%1 被禁言").arg(e.users.front().username.c_str()) : QString(u8"%1 被解除禁言").arg(e.users.front().username.c_str());

		w->listWidget()->insertCenterWidget(text);

		_conModel->setMessageRole(e.groupId, text);
	});

}


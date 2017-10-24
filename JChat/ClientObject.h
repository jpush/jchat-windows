#pragma once

#include <mutex>
#include <QObject>
#include <QDateTime>
#include <QString>
#include <QPixmap>
#include <QNetworkAccessManager>

#include <Jmcpp/Client.h>

#include "await.h"
#include "MetaTypes.h"

#include "User.h"

namespace JChat
{
	class ClientObject :public QObject, public Jmcpp::Client, public std::enable_shared_from_this<ClientObject>
	{
		Q_OBJECT
	public:

		ClientObject(Jmcpp::Configuration const& cfg);

		~ClientObject();


		User* createUser(Jmcpp::UserId const& userId, QObject* parent = nullptr)
		{
			Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
			std::unique_lock<std::mutex> locker(_lock);
			auto iter = _userhandles.find(userId);
			if(iter != _userhandles.end())
			{
				return new User(iter->second, parent);
			}

			auto handle = std::make_shared<UserHandle>(shared_from_this(), userId);

			_userhandles.insert_or_assign(userId, handle);
			return new User(handle, parent);
		}


		////
		pplx::task<void>				getAllData();


		pplx::task<QString>				getUserAvatarFilePath(Jmcpp::UserId userId, std::string avatarMediaId);


		pplx::task<Jmcpp::UserInfo>		getCacheUserInfo(Jmcpp::UserId userId, bool update = false);
		pplx::task<Jmcpp::GroupInfo>	getCacheGroupInfo(Jmcpp::GroupId groupId, bool update = false);

		pplx::task<QPixmap>				getCacheUserAvatar(Jmcpp::UserId userId, std::string avatarMediaId = std::string(), bool update = false);
		pplx::task<QPixmap>				getCacheGroupAvatar(Jmcpp::GroupId groupId, std::string avatarMediaId = std::string(), bool update = false);


		pplx::task<QPixmap>				getCacheAvatar(Jmcpp::ConversationId const&, std::string avatarMediaId = std::string(), bool update = false);


		pplx::task<QString>				getUserDisplayName(Jmcpp::UserId userId, bool update = false);
		pplx::task<QString>				getGroupDisplayName(Jmcpp::GroupId groupId, bool update = false);

		pplx::task<QString>				getDisplayName(Jmcpp::ConversationId const& conId, bool update = false);

		QPixmap							getConversationImage(Jmcpp::ConversationId const& conId);


		pplx::task<QPixmap>				getStaticMap(Jmcpp::LocationContent const& content);

		//////////////////////////////////////////////////////////////////////////

		QString					imageFilePath(Jmcpp::ImageContent const& content) const;
		QString					imageFilePath(QString const& fileName) const;
		QString					filePath(Jmcpp::FileContent const& content) const;


		pplx::task<QString>		getImage(Jmcpp::ImageContent const& content);

		pplx::task<QString>		getFile(Jmcpp::FileContent const& content, std::function<void(int)> progress = nullptr);


		pplx::task<QByteArray>	getResourceData(std::string const& mediaId, std::function<void(int)> progress = nullptr);


		//////////////////////////////////////////////////////////////////////////

		bool				isFriend(Jmcpp::UserId const& userId);

		bool				isNotDisturb(Jmcpp::UserId const& userId);

		bool				isNotDisturb(Jmcpp::GroupId groupId);

		bool				isNotDisturb(Jmcpp::ConversationId const& conId);


		bool				isShield(Jmcpp::GroupId groupId);

		bool				isInBlackList(Jmcpp::UserId const& userId);

		bool				getFriendRemark(Jmcpp::UserId const& userId);

		//////////////////////////////////////////////////////////////////////////
		// hide base

		pplx::task<void>			updateFriendRemark(Jmcpp::UserId const& userId, std::string const& remark, std::string const& remarkOthers = {});

		// 添加signal

		pplx::task<void>			setGroupShield(Jmcpp::GroupId groupId, bool on);

		pplx::task<void>			setNotDisturb(Jmcpp::GroupId groupId, bool on);

		pplx::task<void>			setNotDisturb(Jmcpp::UserId userId, bool on);


		pplx::task<void>			addToBlackList(std::vector<Jmcpp::UserId> usernames);
		pplx::task<void>			removeFromBlackList(std::vector<Jmcpp::UserId> usernames);

		pplx::task<Jmcpp::MessagePtr> sendMessage(Jmcpp::MessagePtr const& msg) const;

		pplx::task<void>	addFriend(Jmcpp::UserId userId, std::string const& requestMsg);

		pplx::task<void>	passAddFriend(Jmcpp::UserId const& userId);
	public:


		pplx::task<QByteArray> _download(QUrl const& url, std::function<void(int)> progress = nullptr);


		Q_SIGNAL void userLogined();

		Q_SIGNAL void disconnected();

		Q_SIGNAL void messageSent(Jmcpp::MessagePtr const& msg) const;

		Q_SIGNAL void messageReceived(Jmcpp::MessagePtr const& msg);

		Q_SIGNAL void messagesReceived(std::vector<Jmcpp::MessagePtr> const& msgs);

		Q_SIGNAL void userInfoUpdated(Jmcpp::UserId const& userId);

		Q_SIGNAL void selfInfoUpdated(Jmcpp::UserId const& userId);

		Q_SIGNAL void groupInfoUpdated(Jmcpp::GroupId groupId);

		Q_SIGNAL void friendListUpdated(Jmcpp::UserInfoList const&);

		Q_SIGNAL void groupListUpdated(Jmcpp::GroupInfoList const&);

		Q_SIGNAL void friendsAdded(Jmcpp::UserInfoList const&);

		Q_SIGNAL void friendsRemoved(Jmcpp::UserIdList const&);

		Q_SIGNAL void notDisturbChanged(Jmcpp::ConversationId const& conId, bool on);
		Q_SIGNAL void groupShieldChanged(Jmcpp::GroupId groupId, bool on);
		Q_SIGNAL void blackListChanged(Jmcpp::UserId const& userId, bool added);

		// events

		Q_SIGNAL void forceLogoutEvent(Jmcpp::ForceLogoutEvent const& e);

		Q_SIGNAL void messageRetracted(Jmcpp::MessageRetractedEvent const& e);

		Q_SIGNAL void requestAddFriendSent(Jmcpp::UserId const& userId, bool isFriend);

		Q_SIGNAL void requestAddFriendEvent(Jmcpp::RequestAddFriendEvent const& e);
		Q_SIGNAL void passAddFriendEvent(Jmcpp::PassAddFriendEvent const& e);
		Q_SIGNAL void rejectAddFriendEvent(Jmcpp::RejectAddFriendEvent const& e);

		Q_SIGNAL void multiFriendAddedEvent(Jmcpp::MultiFriendAddedEvent const& e);
		Q_SIGNAL void multiFriendRemovedEvent(Jmcpp::MultiFriendRemovedEvent const& e);
		Q_SIGNAL void multiFriendRemarkUpdatedEvent(Jmcpp::MultiFriendRemarkUpdatedEvent const& e);

		//

		Q_SIGNAL void groupCreatedEvent(Jmcpp::GroupCreatedEvent const&e);
		Q_SIGNAL void groupInfoUpdatedEvent(Jmcpp::GroupInfoUpdatedEvent const&e);

		Q_SIGNAL void addedToGroupEvent(Jmcpp::AddedToGroupEvent const&e);

		Q_SIGNAL void leavedGroupEvent(Jmcpp::LeavedGroupEvent const&e);
		Q_SIGNAL void removedFromGroupEvent(Jmcpp::RemovedFromGroupEvent const&e);


		Q_SIGNAL void multiUnreadMsgCountChangedEvent(Jmcpp::MultiUnreadMsgCountChangedEvent const&e);

		Q_SIGNAL void receiptsUpdatedEvent(Jmcpp::ReceiptsUpdatedEvent const&e);


		static Jmcpp::Configuration getSDKConfig();

		static Jmcpp::Authorization getAuthorization();

		static QDir storageRootPath(){ return _storageRootPath; }

		QSqlDatabase & memoryDb()
		{
			return _memoryDb;
		}
	protected:

		void init();

		void initPath();

		void initDB();

		void initDB(QSqlDatabase const& db);

		pplx::task<QString> _getGroupDummyName(Jmcpp::GroupId groupId);


		pplx::task<void> updateFriendList();

		pplx::task<void> updateGroupList();

		pplx::task<void> getNotDisturbInfo();


		pplx::task<void> getGroupShieldsInfo();

		pplx::task<void> updateBlackList();



	private:
		template<class T>
		void onEvent(T const& e){ qDebug() << "*****" << typeid(e).name(); }

		void onEvent(Jmcpp::ForceLogoutEvent const& e);

		void onEvent(Jmcpp::MessageRetractedEvent const& e);

		void onEvent(Jmcpp::RequestAddFriendEvent const& e);

		void onEvent(Jmcpp::PassAddFriendEvent const& e);
		void onEvent(Jmcpp::RejectAddFriendEvent const& e);

		void onEvent(Jmcpp::RemovedByFriendEvent const& e);

		void onEvent(Jmcpp::UserUpdatedEvent const& e);

		None onEvent(Jmcpp::UserInfoUpdatedEvent const& e);

		//group
		void onEvent(Jmcpp::GroupCreatedEvent const& e);

		void onEvent(Jmcpp::LeavedGroupEvent const& e);

		void onEvent(Jmcpp::AddedToGroupEvent const& e);

		void onEvent(Jmcpp::RemovedFromGroupEvent const& e);

		None onEvent(Jmcpp::GroupInfoUpdatedEvent const& e);
		//////////////////////////////////////////////////////////////////////////

		void onEvent(Jmcpp::MultiFriendAddedEvent const& e)
		{
			emit multiFriendAddedEvent(e);
		}
		void onEvent(Jmcpp::MultiFriendRemovedEvent const& e)
		{
			emit multiFriendRemovedEvent(e);
		}

		void onEvent(Jmcpp::MultiFriendRemarkUpdatedEvent const& e);

		void onEvent(Jmcpp::MultiNoDisturbChangedEvent const& e);
		void onEvent(Jmcpp::MultiBlackListChangedEvent const& e)
		{
			std::unique_lock<std::mutex> locker(_lock);
			if(e.added)
			{
				_blackList.insert(e.user);
			}
			else
			{
				_blackList.erase(e.user);
			}

			emit blackListChanged(e.user, e.added);

		}
		void onEvent(Jmcpp::MultiGroupShieldChangedEvent const& e);


		//////////////////////////////////////////////////////////////////////////
		void onEvent(Jmcpp::MultiUnreadMsgCountChangedEvent const& e);
		void onEvent(Jmcpp::ReceiptsUpdatedEvent const& e);
	private:
		std::mutex								_lock;

		std::map<Jmcpp::UserId, std::shared_ptr<UserHandle> > _userhandles;


		std::unordered_map<std::string, pplx::task<QByteArray > > _resourceTask;

		std::map<Jmcpp::UserId, Jmcpp::UserInfo> _friendInfos;

		std::map<Jmcpp::UserId, Jmcpp::UserInfo> _userInfos;

		std::map<Jmcpp::GroupId, Jmcpp::GroupInfo>		_groupInfos;


		std::set<Jmcpp::UserId>	_blackList;

		std::set<Jmcpp::UserId>	_notDisturbUser;
		std::set<Jmcpp::GroupId>_notDisturbGroup;

		std::set<Jmcpp::GroupId>_shieldGroup;

		int						globalNotDisturb = 0;

		static QDir						_storageRootPath;
		QDir							_userRootPath;
		QDir							_avatarCachePath;
		QDir							_fileReceivedPath;
		QDir							_imageReceivedPath;

		QSqlDatabase					_memoryDb;

		QThread*						_netThread;
		QNetworkAccessManager*			_manager;
	};

	using ClientObjectPtr = std::shared_ptr<ClientObject>;
}



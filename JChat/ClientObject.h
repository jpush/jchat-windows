#pragma once

#include <mutex>
#include <QObject>
#include <QDateTime>
#include <QString>
#include <QPixmap>
#include <QNetworkAccessManager>

#include <Jmcpp/Client.h>

#include <boost/callable_traits.hpp>

#include "await.h"
#include "MetaTypes.h"


namespace JChat
{
	class ClientObject :public QObject, public Jmcpp::Client, public std::enable_shared_from_this<ClientObject>
	{
		Q_OBJECT
	public:

		ClientObject(Jmcpp::Configuration const& cfg);

		~ClientObject();

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

		Jmcpp::RoomId		getCurrentRoomId()
		{
			if(!_roomId.get())
			{
				KeyValueT kv;
				kv.key = "currentRoomId";
				auto err = qx::dao::fetch_by_id(kv);
				bool ok = false;
				if(auto v = kv.value.toInt(&ok); ok)
				{
					_roomId = v;
				}
			}
			return _roomId;
		}

		void		setCurrentRoomId(Jmcpp::RoomId roomId)
		{
			_roomId = roomId;
			KeyValueT kv;
			kv.key = "currentRoomId";
			kv.value = _roomId.get();
			qx::dao::save(kv);
		}

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


		//
		Q_SIGNAL void onEventSignal(Jmcpp::Event ev);

		//
		Q_SIGNAL void userLogined();

		Q_SIGNAL void disconnected();

		Q_SIGNAL void messageSent(Jmcpp::MessagePtr const& msg) const;

		Q_SIGNAL void sendMessageFailed(Jmcpp::MessagePtr const& msg) const;

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
		Q_SIGNAL void requestAddFriendSent(Jmcpp::UserId const& userId, bool isFriend);



		static Jmcpp::Configuration getSDKConfig();

		static Jmcpp::Authorization getAuthorization();

		static QDir storageRootPath();

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


		template<class E, class F>
		struct VisitSlot
		{
			std::decay_t<F> _f;

			void operator()(E const& ev) const
			{
				_f(ev);
			}

			template<class U>
			void operator()(U const&) const
			{// discard
			}
		};

		template<class E, class F>
		static auto visitSlot(F&& f)
		{
			return VisitSlot<E, F>{std::forward<F>(f)};
		}

	public:
		template<class E, class F, class = std::enable_if_t< !std::is_member_function_pointer< std::decay_t<F> >::value >>
		auto onEvent(QObject* context, F&& f) const
		{
			return connect(this, &ClientObject::onEventSignal, context, [f = std::forward<F>(f)](Jmcpp::Event const& ev) mutable
			{
				std::visit(visitSlot<E>(std::forward<F>(f)), ev);
			});
		}

		template<int..., class F = void, class E = std::tuple_element_t<1, boost::callable_traits::args_t<decltype(&std::decay_t<F>::operator())>> >
		auto onEvent(QObject* context, F&& f) const
		{
			return connect(this, &ClientObject::onEventSignal, context, [f = std::forward<F>(f)](Jmcpp::Event const& ev) mutable
			{
				std::visit(visitSlot<std::decay_t<E>>(std::forward<F>(f)), ev);
			});
		}


		template<int..., class E, class R, class C, class = std::enable_if_t< std::is_base_of<QObject, C>::value >>
		auto onEvent(const std::remove_reference_t<C>* obj, R(C::*pmf)(E)) const
		{
			return connect(this, &ClientObject::onEventSignal, obj, [obj, pmf](Jmcpp::Event const& ev) mutable
			{
				std::visit(visitSlot<std::decay_t<E>>(std::bind(pmf, const_cast<C*>(obj), std::placeholders::_1)), ev);
			});
		}

	private:
		template<class T>
		None doVisitEvent(T const&e, Jmcpp::Event event)
		{
			if constexpr(std::is_void_v<decltype(visitEvent(std::declval<T const&>()))>)
			{
				visitEvent(e);
			}
			else
			{
				co_await visitEvent(e);
			}

			Q_EMIT onEventSignal(std::move(event));
			
			co_await std::experimental::suspend_never();
		}


		template<class T>
		void visitEvent(T const& e)
		{
			qDebug() << "*****" << typeid(e).name();
		}


		void visitEvent(Jmcpp::PassAddFriendEvent const& e);

		void visitEvent(Jmcpp::RemovedByFriendEvent const& e);


		pplx::task<void> visitEvent(Jmcpp::UserInfoUpdatedEvent const& e);


		pplx::task<void> visitEvent(Jmcpp::GroupInfoUpdatedEvent const& e);


		void visitEvent(Jmcpp::MultiFriendRemarkUpdatedEvent const& e);

		void visitEvent(Jmcpp::MultiNoDisturbChangedEvent const& e);
		void visitEvent(Jmcpp::MultiBlackListChangedEvent const& e);
		void visitEvent(Jmcpp::MultiGroupShieldChangedEvent const& e);


	private:
		std::mutex								_lock;


		std::unordered_map<std::string, pplx::task<QByteArray > > _resourceTask;

		std::map<Jmcpp::UserId, Jmcpp::UserInfo> _friendInfos;

		std::map<Jmcpp::UserId, Jmcpp::UserInfo> _userInfos;

		std::map<Jmcpp::GroupId, Jmcpp::GroupInfo>		_groupInfos;


		std::set<Jmcpp::UserId>	_blackList;

		std::set<Jmcpp::UserId>	_notDisturbUser;
		std::set<Jmcpp::GroupId>_notDisturbGroup;

		std::set<Jmcpp::GroupId>_shieldGroup;

		int						globalNotDisturb = 0;


		Jmcpp::RoomId			_roomId;

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



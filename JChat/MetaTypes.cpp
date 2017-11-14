#include "MetaTypes.h"


namespace
{
	void registerTypes()
	{

		qRegisterMetaType<int64_t>("int64_t");


		qRegisterMetaType<Jmcpp::UserId>("Jmcpp::UserId");
		qRegisterMetaType<Jmcpp::GroupId>("Jmcpp::GroupId");
		qRegisterMetaType<Jmcpp::RoomId>("Jmcpp::RoomId");

		qRegisterMetaType<Jmcpp::UserIdList>("Jmcpp::UserIdList");

		qRegisterMetaType<Jmcpp::ConversationId>("Jmcpp::ConversationId");
		qRegisterMetaType<Jmcpp::ChatId>("Jmcpp::ChatId");


		qRegisterMetaType<std::vector<Jmcpp::MessagePtr>>("std::vector<Jmcpp::MessagePtr>");
		qRegisterMetaType<Jmcpp::MessagePtr>("Jmcpp::MessagePtr");

		qRegisterMetaType<Jmcpp::UserInfoList>("Jmcpp::UserInfoList");

		qRegisterMetaType<Jmcpp::GroupInfoList>("Jmcpp::GroupInfoList");

		qRegisterMetaType<Jmcpp::ForceLogoutEvent >("Jmcpp::ForceLogoutEvent");
		qRegisterMetaType<Jmcpp::RequestAddFriendEvent>("Jmcpp::RequestAddFriendEvent");
		qRegisterMetaType<Jmcpp::RejectAddFriendEvent>("Jmcpp::RejectAddFriendEvent");
		qRegisterMetaType<Jmcpp::PassAddFriendEvent >("Jmcpp::PassAddFriendEvent");
		qRegisterMetaType<Jmcpp::RemovedByFriendEvent >("Jmcpp::RemovedByFriendEvent");

		qRegisterMetaType<Jmcpp::UserUpdatedEvent  >("Jmcpp::UserUpdatedEvent");
		qRegisterMetaType<Jmcpp::GroupCreatedEvent >("Jmcpp::GroupCreatedEvent");
		qRegisterMetaType<Jmcpp::LeavedGroupEvent >("Jmcpp::LeavedGroupEvent");
		qRegisterMetaType<Jmcpp::AddedToGroupEvent>("Jmcpp::AddedToGroupEvent");

		qRegisterMetaType<Jmcpp::GroupMemberSilentChangedEvent>("Jmcpp::GroupMemberSilentChangedEvent");

		qRegisterMetaType<Jmcpp::RemovedFromGroupEvent>("Jmcpp::RemovedFromGroupEvent");
		qRegisterMetaType<Jmcpp::GroupInfoUpdatedEvent>("Jmcpp::GroupInfoUpdatedEvent");
		qRegisterMetaType<Jmcpp::NoDisturbUpdatedEvent>("Jmcpp::NoDisturbUpdatedEvent");
		qRegisterMetaType<Jmcpp::BlackListUpdatedEvent>("Jmcpp::BlackListUpdatedEvent");
		qRegisterMetaType<Jmcpp::MessageRetractedEvent>("Jmcpp::MessageRetractedEvent");


		qRegisterMetaType<Jmcpp::MultiFriendAddedEvent>("Jmcpp::MultiFriendAddedEvent");
		qRegisterMetaType<Jmcpp::MultiFriendRemovedEvent>("Jmcpp::MultiFriendRemovedEvent");
		qRegisterMetaType<Jmcpp::MultiFriendRemarkUpdatedEvent>("Jmcpp::MultiFriendRemarkUpdatedEvent");

		qRegisterMetaType<Jmcpp::MultiNoDisturbChangedEvent>("Jmcpp::MultiNoDisturbChangedEvent");
		qRegisterMetaType<Jmcpp::MultiBlackListChangedEvent>("Jmcpp::MultiBlackListChangedEvent");
		qRegisterMetaType<Jmcpp::MultiGroupShieldChangedEvent>("Jmcpp::MultiGroupShieldChangedEvent");

		qRegisterMetaType<Jmcpp::ReceiptsUpdatedEvent>("Jmcpp::ReceiptsUpdatedEvent");

		qRegisterMetaType<Jmcpp::MultiFriendRemovedEvent >("Jmcpp::FriendRemovedEvent");

		qRegisterMetaType<Jmcpp::MultiUnreadMsgCountChangedEvent >("Jmcpp::MultiUnreadMsgCountChangedEvent");

		qRegisterMetaType<Jmcpp::TransCommandEvent >("Jmcpp::TransCommandEvent");
	}

	Q_CONSTRUCTOR_FUNCTION(registerTypes);
}

QX_REGISTER_COMPLEX_CLASS_NAME_CPP(JChat::KeyValueT, JChatKeyValueT)

namespace qx {
	template <>
	void register_class(QxClass<JChat::KeyValueT> & t)
	{
		using JChat::KeyValueT;
		t.setName("KeyValueT");
		t.id(&KeyValueT::key, "key");
		t.data(&KeyValueT::value, "value");
	}
}

QX_REGISTER_COMPLEX_CLASS_NAME_CPP(JChat::UserInfo, JChatUserInfo)

namespace qx {
	template <>
	void register_class(QxClass<JChat::UserInfo> & t)
	{
		using JChat::UserInfo;
		t.setName("UserInfoT");
		t.id(&UserInfo::userId, "username|appkey");

		t.data(&UserInfo::nickname, "nickname");
		t.data(&UserInfo::avatar, "avatar");
		t.data(&UserInfo::remark, "remark");
		t.data(&UserInfo::birthday, "birthday");
		t.data(&UserInfo::gender, "gender");
		t.data(&UserInfo::signature, "signature");
		t.data(&UserInfo::region, "region");
		t.data(&UserInfo::address, "address");
		t.data(&UserInfo::mtime, "mtime");
	}
}

QX_REGISTER_COMPLEX_CLASS_NAME_CPP(JChat::ConversationT, JChatConversationT)

namespace qx {
	template <>
	void register_class(QxClass<JChat::ConversationT> & t)
	{
		using JChat::ConversationT;
		t.setName("ConversationT");
		t.id(&ConversationT::conId, "username|appkey|groupId|roomId");

		t.data(&ConversationT::lastChatTime, "lastChatTime");
		t.data(&ConversationT::lastMessageStr, "lastMessageStr");

		t.data(&ConversationT::unreadMsgCount, "unreadMsgCount");

		t.data(&ConversationT::sticktopTime, "sticktopTime");

		t.data(&ConversationT::deleted, "deleted");
	}
}


QX_REGISTER_COMPLEX_CLASS_NAME_CPP(JChat::Account, JChatAccount)
namespace qx {
	template <>
	void register_class(QxClass<JChat::Account> & t)
	{
		using JChat::Account;
		t.setName("Account");

		t.id(&Account::username, "username");
		t.data(&Account::password, "password");
		t.data(&Account::lastTime, "lastTime");

	}
}


QX_REGISTER_COMPLEX_CLASS_NAME_CPP(JChat::FriendEventT, JChatFriendEventT)
namespace qx {
	template <>
	void register_class(QxClass<JChat::FriendEventT> & t)
	{
		using JChat::FriendEventT;
		t.setName("FriendEventT");
		t.id(&FriendEventT::id, "id");
		t.data(&FriendEventT::username, "username");
		t.data(&FriendEventT::appkey, "appkey");

		t.data(&FriendEventT::isOutgoing, "isOutgoing");
		t.data(&FriendEventT::message, "message");

		t.data(&FriendEventT::hasRead, "hasRead");

		t.data(&FriendEventT::status, "status");
		t.data(&FriendEventT::time, "time");
	}
}

QX_REGISTER_COMPLEX_CLASS_NAME_CPP(JChat::GroupEventT, JChatGroupEventT)
namespace qx {
	template <>
	void register_class(QxClass<JChat::GroupEventT> & t)
	{
		using JChat::GroupEventT;
		t.setName("GroupEventT");
		t.id(&GroupEventT::id, "id");
		t.data(&GroupEventT::eventId, "eventId");
		t.data(&GroupEventT::groupId, "groupId");

		t.data(&GroupEventT::isReject, "isReject");
		t.data(&GroupEventT::bySelf, "bySelf");

		t.data(&GroupEventT::hasRead, "hasRead");

		t.data(&GroupEventT::fromUser, "fromUser");
		t.data(&GroupEventT::user, "user");

		t.data(&GroupEventT::status, "status");
		t.data(&GroupEventT::time, "time");
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template <>
QVariant qx::cvt::to_variant(const Jmcpp::UserId & t, const QString & format, int index, context::ctx_type ctx)
{
	switch(index)
	{
		case -1:
		{
			QByteArray data;
			QDataStream ds(&data, QIODevice::WriteOnly);
			ds << t.username << t.appKey;
			return data;
		}break;
		case 0:
		{
			return to_variant(t.username, format, -1, ctx);
		}break;
		case 1:
		{
			return to_variant(t.appKey, format, -1, ctx);
		}break;
		default:
			break;
	}
	Q_ASSERT(false);
	return {};
}

template <>
qx_bool qx::cvt::from_variant(const QVariant & v, Jmcpp::UserId & t, const QString & format, int index, context::ctx_type ctx)
{
	switch(index)
	{
		case -1:
		{
			auto data = v.toByteArray();
			QDataStream ds(data);
			ds >> t.username >> t.appKey;
			return true;
		}break;
		case 0:
		{
			return from_variant(v, t.username, format, -1, ctx);
		}break;
		case 1:
		{
			return from_variant(v, t.appKey, format, -1, ctx);
		}break;
		default:
			break;
	}
	Q_ASSERT(false);
	return false;
}


//////////////////////////////////////////////////////////////////////////
template <>
QVariant qx::cvt::to_variant(const Jmcpp::ConversationId & t, const QString & format, int index, context::ctx_type ctx)
{
	switch(index)
	{
		case -1:
		{
			QByteArray data;
			QDataStream ds(&data, QIODevice::WriteOnly);
			ds << t.getUserId().username << t.getUserId().appKey << t.getGroupId().get() << t.getRoomId().get();
			return data;
		}break;
		case 0:
		{
			return to_variant(t.getUserId().username, format, -1, ctx);
		}break;
		case 1:
		{
			return to_variant(t.getUserId().appKey, format, -1, ctx);
		}break;
		case 2:
		{
			return to_variant(t.getGroupId().get(), format, -1, ctx);
		}break;
		case 3:
		{
			return to_variant(t.getRoomId().get(), format, -1, ctx);
		}break;
		default:
			break;
	}
	Q_ASSERT(false);
	return {};
}

template <>
qx_bool qx::cvt::from_variant(const QVariant & v, Jmcpp::ConversationId & t, const QString & format, int index, context::ctx_type ctx)
{
	switch(index)
	{
		case -1:
		{
			auto data = v.toByteArray();
			QDataStream ds(data);
			int64_t groupId = 0, roomId = 0;
			std::string username, appkey;
			ds >> username >> appkey >> groupId >> roomId;
			if(groupId)
			{
				t.setGroupId(groupId);
			}
			else if(roomId)
			{
				t.setRoomId(roomId);
			}
			else
			{
				t.setUserId({ username,appkey });
			}
			return true;
		}break;

		case 0:
		{
			std::string username, appkey;
			from_variant(v, username, format, -1, ctx);
			if(!t.isGroup() && !t.isRoom())
			{
				t.setUsername(username);
			}
			return true;
		}break;
		case 1:
		{
			std::string appkey;
			from_variant(v, appkey, format, -1, ctx);
			if(!t.isGroup() && !t.isRoom())
			{
				t.setAppkey(appkey);
			}
			return true;
		}break;
		case 2:
		{
			int64_t groupId = 0;
			from_variant(v, groupId, format, -1, ctx);
			if(groupId)
			{
				t.setGroupId(groupId);
			}
			return true;
		}break;
		case 3:
		{
			int64_t roomId = 0;
			from_variant(v, roomId, format, -1, ctx);
			if(roomId)
			{
				t.setRoomId(roomId);
			}
			return true;
		}break;
		default:
			break;
	}
	Q_ASSERT(false);
	return false;
}


//////////////////////////////////////////////////////////////////////////
template <>
QVariant qx::cvt::to_variant(const Jmcpp::GroupId & t, const QString & format, int index, context::ctx_type ctx)
{
	switch(index)
	{
		case -1:
		{
			QByteArray data;
			QDataStream ds(&data, QIODevice::WriteOnly);
			ds << t.get();
			return data;
		}break;
		case 0:
		{
			return to_variant(t.get(), format, -1, ctx);
		}break;
		default:
			break;
	}
	Q_ASSERT(false);
	return {};
}

template <>
qx_bool qx::cvt::from_variant(const QVariant & v, Jmcpp::GroupId & t, const QString & format, int index, context::ctx_type ctx)
{
	switch(index)
	{
		case -1:
		{
			auto data = v.toByteArray();
			QDataStream ds(data);
			int64_t groupId = 0;
			ds >> groupId;
			if(groupId)
			{
				t = groupId;
			}
			return true;
		}break;
		case 0:
		{
			int64_t groupId = 0;
			from_variant(v, groupId, format, -1, ctx);
			t = groupId;
			return true;
		}break;
		default:
			break;
	}
	Q_ASSERT(false);
	return false;
}


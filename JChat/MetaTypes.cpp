#include "MetaTypes.h"


namespace
{
	void registerTypes()
	{

		qRegisterMetaType<int64_t>("int64_t");


		qRegisterMetaType<Jmcpp::UserId>("Jmcpp::UserId");
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

	}

	Q_CONSTRUCTOR_FUNCTION(registerTypes);
}



QX_REGISTER_COMPLEX_CLASS_NAME_CPP(JChat::UserInfo, JChatUserInfo)

namespace qx {
	template <>
	void register_class(QxClass<JChat::UserInfo> & t)
	{
		using JChat::UserInfo;
		t.setName("UserInfo");
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


QX_REGISTER_COMPLEX_CLASS_NAME_CPP(JChat::Conversation, JChatConversation)

namespace qx {
	template <>
	void register_class(QxClass<JChat::Conversation> & t)
	{
		using JChat::Conversation;
		t.setName("Conversation");
		t.id(&Conversation::conId, "groupId|username|appkey");

		t.data(&Conversation::lastChatTime, "lastChatTime");
		t.data(&Conversation::lastMessageStr, "lastMessageStr");

		t.data(&Conversation::unreadMsgCount, "unreadMsgCount");

		t.data(&Conversation::sticktopTime, "sticktopTime");

		t.data(&Conversation::deleted, "deleted");
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


QX_REGISTER_COMPLEX_CLASS_NAME_CPP(JChat::FriendEventTable, JChatFriendEventTable)
namespace qx {
	template <>
	void register_class(QxClass<JChat::FriendEventTable> & t)
	{
		using JChat::FriendEventTable;
		t.setName("FriendEventTable");
		t.id(&FriendEventTable::id, "id");
		t.data(&FriendEventTable::username, "username");
		t.data(&FriendEventTable::appkey, "appkey");

		t.data(&FriendEventTable::isOutgoing, "isOutgoing");
		t.data(&FriendEventTable::message, "message");

		t.data(&FriendEventTable::hasRead, "hasRead");

		t.data(&FriendEventTable::status, "status");
		t.data(&FriendEventTable::time, "time");
	}
}




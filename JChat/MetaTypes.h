#pragma once

#include <QVariant>

#include <Jmcpp/Client.h>
#include <QxOrm.h>


Q_DECLARE_METATYPE(std::string)

Q_DECLARE_METATYPE(Jmcpp::UserId)
Q_DECLARE_METATYPE(Jmcpp::GroupId)
Q_DECLARE_METATYPE(Jmcpp::UserIdList)

Q_DECLARE_METATYPE(Jmcpp::UserInfo)
Q_DECLARE_METATYPE(Jmcpp::ConversationId)
Q_DECLARE_METATYPE(Jmcpp::GroupInfo)
Q_DECLARE_METATYPE(Jmcpp::UserInfoLite)
Q_DECLARE_METATYPE(Jmcpp::GroupMember)


Q_DECLARE_METATYPE(Jmcpp::UserInfoList)
Q_DECLARE_METATYPE(Jmcpp::GroupInfoList)


Q_DECLARE_METATYPE(Jmcpp::NullContent)
Q_DECLARE_METATYPE(Jmcpp::TextContent)
Q_DECLARE_METATYPE(Jmcpp::ImageContent)
Q_DECLARE_METATYPE(Jmcpp::FileContent)

Q_DECLARE_METATYPE(Jmcpp::MessageContent)

Q_DECLARE_METATYPE(Jmcpp::MessagePtr)

Q_DECLARE_METATYPE(std::vector<Jmcpp::MessagePtr>)


Q_DECLARE_METATYPE(Jmcpp::ForceLogoutEvent)
Q_DECLARE_METATYPE(Jmcpp::RequestAddFriendEvent)
Q_DECLARE_METATYPE(Jmcpp::RejectAddFriendEvent)
Q_DECLARE_METATYPE(Jmcpp::PassAddFriendEvent)
Q_DECLARE_METATYPE(Jmcpp::RemovedByFriendEvent)
Q_DECLARE_METATYPE(Jmcpp::UserUpdatedEvent)
Q_DECLARE_METATYPE(Jmcpp::GroupCreatedEvent)
Q_DECLARE_METATYPE(Jmcpp::LeavedGroupEvent)
Q_DECLARE_METATYPE(Jmcpp::AddedToGroupEvent)

Q_DECLARE_METATYPE(Jmcpp::RemovedFromGroupEvent)
Q_DECLARE_METATYPE(Jmcpp::GroupInfoUpdatedEvent)
Q_DECLARE_METATYPE(Jmcpp::NoDisturbUpdatedEvent)
Q_DECLARE_METATYPE(Jmcpp::BlackListUpdatedEvent)
Q_DECLARE_METATYPE(Jmcpp::MessageRetractedEvent)

Q_DECLARE_METATYPE(Jmcpp::MultiFriendAddedEvent)
Q_DECLARE_METATYPE(Jmcpp::MultiFriendRemovedEvent)
Q_DECLARE_METATYPE(Jmcpp::MultiFriendRemarkUpdatedEvent)


Q_DECLARE_METATYPE(Jmcpp::MultiNoDisturbChangedEvent)
Q_DECLARE_METATYPE(Jmcpp::MultiBlackListChangedEvent)
Q_DECLARE_METATYPE(Jmcpp::MultiGroupShieldChangedEvent)

Q_DECLARE_METATYPE(Jmcpp::ReceiptsUpdatedEvent)

namespace qx::cvt
{

	template <>
	QVariant to_variant(const Jmcpp::UserId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx);

	template <>
	qx_bool from_variant(const QVariant & v, Jmcpp::UserId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx);


	template <>
	QVariant to_variant(const Jmcpp::ConversationId & t, const QString & format, int index , qx::cvt::context::ctx_type ctx );

	template <>
	qx_bool from_variant(const QVariant & v, Jmcpp::ConversationId & t, const QString & format, int index , qx::cvt::context::ctx_type ctx );
}




namespace JChat
{
	using Jmcpp::UserInfo;

	struct Conversation
	{
		using key_t = Jmcpp::ConversationId;

		Jmcpp::ConversationId	conId;

		int						unreadMsgCount = 0;
		QDateTime				lastChatTime = QDateTime::currentDateTime();
		QString					lastMessageStr;

		QDateTime				sticktopTime;
		bool					deleted = false;
	};

	struct Account
	{
		QString username;
		QString password;
		QDateTime lastTime;
	};

	struct FriendEventTable
	{
		enum Status
		{
			undone,
			passed,
			rejected,
		};

		int64_t			id = 0;
		std::string		username, appkey;
		bool			isOutgoing = true;
		bool			hasRead = false;

		QString			message;
		Status			status = undone;
		QDateTime		time;
	};
}


QX_REGISTER_PRIMARY_KEY(JChat::UserInfo, Jmcpp::UserId)
QX_REGISTER_COMPLEX_CLASS_NAME_HPP_EXPORT_DLL(JChat::UserInfo, qx::trait::no_base_class_defined, 0, JChatUserInfo)


QX_REGISTER_PRIMARY_KEY(JChat::Conversation, JChat::Conversation::key_t)
QX_REGISTER_COMPLEX_CLASS_NAME_HPP_EXPORT_DLL(JChat::Conversation, qx::trait::no_base_class_defined, 0, JChatConversation)


QX_REGISTER_PRIMARY_KEY(JChat::Account, QString)
QX_REGISTER_COMPLEX_CLASS_NAME_HPP_EXPORT_DLL(JChat::Account, qx::trait::no_base_class_defined, 0, JChatAccount)

QX_REGISTER_PRIMARY_KEY(JChat::FriendEventTable, int64_t)
QX_REGISTER_COMPLEX_CLASS_NAME_HPP_EXPORT_DLL(JChat::FriendEventTable, qx::trait::no_base_class_defined, 0, JChatFriendEventTable)

///
Q_DECLARE_METATYPE(JChat::Conversation);


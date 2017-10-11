#pragma once

#include <QVariant>

#include <Jmcpp/Client.h>
#include <QxOrm.h>


Q_DECLARE_METATYPE(std::string)

Q_DECLARE_METATYPE(Jmcpp::UserId)
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

namespace qx::cvt::detail{

	template <>
	struct QxConvert_ToString< Jmcpp::UserId >
	{
		static inline QString toString(const Jmcpp::UserId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx)
		{
			switch(index)
			{
				//case -1: return t.toString().data();
				case 0:return	to_string(t.username, format, -1, ctx);
				case 1:return	to_string(t.appKey, format, -1, ctx);
			}
			Q_ASSERT(false);
			return QString();
		}
	};

	template <>
	struct QxConvert_FromString<Jmcpp::UserId>
	{
		static inline qx_bool fromString(const QString & s, Jmcpp::UserId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx)
		{
			switch(index)
			{
				//case -1: t = Jmcpp::UserId::fromString(s.toStdString()); return true;
				case 0:return from_string(s, t.username, format, -1, ctx);
				case 1:return from_string(s, t.appKey, format, -1, ctx);
			}
			Q_ASSERT(false);
			return qx_bool(false);
		}
	};

	template <>
	struct QxConvert_ToVariant<Jmcpp::UserId>
	{
		static inline QVariant toVariant(const Jmcpp::UserId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx)
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
	};

	template <>
	struct QxConvert_FromVariant<Jmcpp::UserId>
	{
		static inline qx_bool fromVariant(const QVariant & v, Jmcpp::UserId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx)
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
	};
}


namespace qx::cvt::detail{

	template <>
	struct QxConvert_ToString< Jmcpp::ConversationId >
	{
		static inline QString toString(const Jmcpp::ConversationId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx)
		{
			switch(index)
			{
				//case -1: return t.isUser() ? t.getUserId().toString().data() : QString::number(t.getGroupId());
				case 0:return to_string(t.getGroupId(), format, -1, ctx);
				case 1:return to_string(t.getUserId().username, format, -1, ctx);
				case 2:return to_string(t.getUserId().appKey, format, -1, ctx);
			}
			Q_ASSERT(false);
			return QString();
		}
	};

	template <>
	struct QxConvert_FromString< Jmcpp::ConversationId >
	{
		static inline qx_bool fromString(const QString & s, Jmcpp::ConversationId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx)
		{
			switch(index)
			{
				//case -1: t = Jmcpp::UserId::fromString(s.toStdString()); return true;
				case 0:
				{
					int64_t groupId = 0;
					from_string(s, groupId, format, -1, ctx);
					t.setGroupId(groupId);
					return true;
				}
				case 1:
				{
					std::string username;
					from_string(s, username, format, -1, ctx);
					t.setUsername(username);
					return true;
				}
				case 2:
				{
					std::string appkey;
					from_string(s, appkey, format, -1, ctx);
					t.setAppkey(appkey);
					return true;
				}
			}
			Q_ASSERT(false);
			return qx_bool(false);
		}
	};

	template <>
	struct QxConvert_ToVariant<Jmcpp::ConversationId>
	{
		static inline QVariant toVariant(const Jmcpp::ConversationId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx)
		{
			switch(index)
			{
				case -1:
				{
					QByteArray data;
					QDataStream ds(&data, QIODevice::WriteOnly);
					ds << t.getGroupId() << t.getUserId().username << t.getUserId().appKey;
					return data;
				}break;
				case 0:
				{
					return to_variant(t.getGroupId(), format, -1, ctx);
				}break;
				case 1:
				{
					return to_variant(t.getUserId().username, format, -1, ctx);
				}break;
				case 2:
				{
					return to_variant(t.getUserId().appKey, format, -1, ctx);
				}break;
				default:
					break;
			}
			Q_ASSERT(false);
			return {};
		}
	};

	template <>
	struct QxConvert_FromVariant<Jmcpp::ConversationId>
	{
		static inline qx_bool fromVariant(const QVariant & v, Jmcpp::ConversationId & t, const QString & format, int index, qx::cvt::context::ctx_type ctx)
		{
			switch(index)
			{
				case -1:
				{
					auto data = v.toByteArray();
					QDataStream ds(data);
					int64_t groupId = 0;
					std::string username, appkey;
					ds >> groupId >> username >> appkey;
					if(groupId)
					{
						t.setGroupId(groupId);
					}
					else
					{
						t.setUserId({ username,appkey });
					}
					return true;
				}break;
				case 0:
				{
					int64_t groupId = 0;
					from_variant(v, groupId, format, -1, ctx);
					if(groupId)
					{
						t.setGroupId(groupId);
					}
					return true;
				}break;
				case 1:
				{
					std::string username, appkey;
					from_variant(v, username, format, -1, ctx);
					if(!t.isGroup())
					{
						t.setUsername(username);
					}
					return true;
				}break;
				case 2:
				{
					std::string appkey;
					from_variant(v, appkey, format, -1, ctx);
					if(!t.isGroup())
					{
						t.setAppkey(appkey);
					}
					return true;
				}break;
				default:
					break;
			}
			Q_ASSERT(false);
			return false;
		}
	};
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


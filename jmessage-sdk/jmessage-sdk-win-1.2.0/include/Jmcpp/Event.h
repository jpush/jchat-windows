#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include <tuple>
#include <map>
#include <string>
#include <optional>
#include <variant>
#include "Types.h"
#include "JmcppExport.h"

namespace Jmcpp
{
	struct EventBase
	{
		int64_t eventId = 0;
		int64_t time = 0;
	};

	struct NullEvent :EventBase{};

	//! 强制下线事件
	/**
	* 收到此事件后,\c Client会强制退出登录,任何后续请求都会失败,重新登录应创建新的Client
	**/
	struct ForceLogoutEvent :EventBase
	{
		enum Reason
		{
			Logined,			///< 用户在别的设备登录
			PasswordChanged,	///< 密码被修改
			UserDisabled,		///< 用户被禁用
			UserDeleted,		///< 用户被删除
			Unknown,			///< 未知
		};
		Reason reason;			//!< 下线原因
	};

	//! 被请求添加好友事件
	struct RequestAddFriendEvent :EventBase
	{
		UserId		fromUser;	///< 好友请求发起用户
		std::string message;	///< 验证消息
		//avatar
	};

	//! 好友添加被拒绝事件
	struct RejectAddFriendEvent :EventBase
	{
		UserId		fromUser;		///< 拒绝者
		std::string rejectReason;	///< 拒绝原因
	};

	//! 好友添加请求被通过
	struct PassAddFriendEvent :EventBase
	{
		UserId fromUser; ///< 同意者
	};


	//!被解除好友关系
	struct RemovedByFriendEvent :EventBase
	{
		UserId fromUser; ///< 发起者
	};

	//! 好友更新事件示例
	struct UserUpdatedEvent :EventBase
	{
		std::string description;
	};


	//! 群创建事件,创建者接收此事件
	struct GroupCreatedEvent :EventBase
	{
		UserId				fromUser;	///< 创建者
		GroupId				groupId;	///< 群Id
		std::string			groupName;	///< 群名称
		std::string			mediaId;	///< 群头像 
		std::vector<UserId> users;		///< 创建者
	};

	//! 群解散事件,所有成员接收此事件
	struct GroupDeletedEvent :EventBase
	{
		GroupId				groupId;	///< 群Id
	};

	//!退出群组事件,所有人接收,包括退群者
	struct LeavedGroupEvent :EventBase
	{
		UserId					fromUser;	///< 退群者
		GroupId					groupId;	///< 群Id
		std::string				groupName;	///< 群名称
		std::string				mediaId;	///< 群头像 
		std::vector<UserId>		users;		///< 退群者
		std::optional<UserId>	newOwner;	///< 如果是群主退群,新群主
	};

	//!加入群组事件,包括被添加的成员和原来的成员
	struct AddedToGroupEvent :EventBase
	{
		GroupId				groupId;	///< 群Id
		std::string			groupName;	///< 群名称
		std::string			mediaId;	///< 群头像 
		UserId				fromUser;	///< 邀请者
		std::vector<UserId> users;		///< 添入的成员
	};

	//! 请求加入群事件,群主/管理员接收此事件
	struct RequestJoinGroupEvent :EventBase
	{
		GroupId				groupId{};	///< 群Id
		bool				bySelf{};	///< 是否是主动加群
		UserIdList			users;		///< 被邀请加入群的用户
		UserId				fromUser;	///< 邀请者/申请者
		std::string			requestMsg;	///< 主动加群验证消息
	};

	//! 加入群请求被拒绝事件,邀请者或申请者接收该事件 \sa Client::joinGroup,Client::rejectJoinGroup
	struct RejectJoinGroupEvent :EventBase
	{
		GroupId				groupId{};	///< 群Id
		bool				bySelf{};	///< 是否是主动加群
		UserIdList			users;		///< 被邀请加入群的用户
		UserId				fromUser;	///< 审核人,群主/管理员
		std::string			rejectReason;///< 拒绝原因
	};

	//! 加入群请求被群主/其他管理员拒绝事件,所有管理员和群主接收该事件 \sa Client::rejectJoinGroup
	struct AdminRejectJoinGroupEvent :EventBase
	{
		GroupId				groupId{};	///< 群Id
		UserIdList			users;		///< 被拒绝的用户
		UserId				fromUser;	///< 审核人,群主/管理员
		int64_t				requestJoinGroupEventId{};	///< 被拒绝的申请入群事件Id 
	};

	//!删除群组成员,群里所有人接收,包括被删除的成员和剩下的成员
	struct RemovedFromGroupEvent :EventBase
	{
		GroupId					groupId;	///< 群Id
		UserId					fromUser;	///< 删除者,群主/管理员
		std::string				groupName;	///< 群名称
		std::string				mediaId;	///< 群头像 
		std::vector<UserId>		users;		///< 被删除的成员
		std::optional<UserId>	newOwner;	///< 新群主
	};

	//! 群成员禁言变化事件,群所有用户接收该事件
	struct GroupMemberSilentChangedEvent :EventBase
	{
		GroupId					groupId;	///< 群Id
		UserId					fromUser;	///< 禁言者,群主/管理员
		[[deprecated]] UserId	owner;		///< 禁言者,群主/管理员
		std::vector<UserId>		users;		///< 禁言成员
		bool					on{};		///< true:禁言;false:关闭禁言
	};


	//! 群信息修改,群里所有人接收该事件
	struct GroupInfoUpdatedEvent :EventBase
	{
		UserId				fromUser;	///< 修改者
		GroupId				groupId;	///< 群Id
		std::vector<UserId> users;		///< 修改者
	};

	//! 群管理员变更事件,群里所有人接收该事件
	struct GroupAdminChangedEvent :EventBase
	{
		GroupId				groupId;		///< 群Id
		UserId				owner;			///< 群主
		UserIdList			users;			///< 添加/移除的管理员
		bool				added = true;	///< 添加/移除管理员
	};

	struct GroupOwnerChangedEvent :EventBase
	{
		GroupId				groupId;		///< 群Id
		UserId				oldOwner;		///< 旧群主
		UserId				newOwner;		///< 新群主
	};


	//!免打扰更新事件
	struct NoDisturbUpdatedEvent :EventBase{};

	//! 黑名单更新事件
	struct BlackListUpdatedEvent :EventBase{};

	//! 群屏蔽更新事件
	struct GroupShieldUpdatedEvent :EventBase{};

	//! 用户信息更新事件
	struct UserInfoUpdatedEvent :EventBase
	{
		UserId	user; //!< 自己或者其他用户的信息更新
	};

	//! 消息被撤回事件
	struct MessageRetractedEvent :EventBase
	{
		UserId					fromUser;	///< 消息撤回方
		std::vector<int64_t>	msgIds;		///< 被撤回的消息id列表
		ConversationId			conId;		///< 消息撤回的会话
	};


	//!多通道好友添加事件
	struct MultiFriendAddedEvent :EventBase
	{
		UserId					user;	///< 好友
		std::string				mediaId;///< 好友头像
	};

	//!多通道好友删除事件
	struct MultiFriendRemovedEvent :EventBase
	{
		UserId					user;	///< 好友
		std::string				mediaId;///< 好友头像
	};

	//!多通道好友备注变化事件
	struct MultiFriendRemarkUpdatedEvent :EventBase
	{
		UserId					user;	///< 好友
		std::string				mediaId;///< 好友头像

		std::string				remark;	///< 备注
		std::string				remarkOther;
	};


	//! 多通道黑名单变化事件
	struct MultiBlackListChangedEvent :EventBase
	{
		UserId					user;			///< 用户
		bool					added = true;	///< 添加或移除
	};


	//! 多通道免打扰变化事件
	struct MultiNoDisturbChangedEvent :EventBase
	{
		ConversationId			conId;			///< 用户或群或空, 为空时,表示全局免打扰
		bool					added = true;	///< 添加或移除
	};


	//! 多通道群屏蔽变化事件
	struct MultiGroupShieldChangedEvent :EventBase
	{
		GroupId					groupId;		///< 群
		bool					added = true;	///< 添加或移除
	};


	//! 多通道会话消息未读数变化事件
	struct MultiUnreadMsgCountChangedEvent :EventBase
	{
		ConversationId			conId;			///< 用户或群
		size_t					unreadMsgCount; ///< 会话未读消息数目
		int64_t					lastMessageTime;///< 已读的最后一条消息时间,消息的time <= lastMessageTime 则消息已读,否则未读
	};


	//! 多通道会话消息回执变化事件
	struct MultiReceiptChangedEvent :EventBase
	{
		ConversationId			conId;			///< 用户或群
		std::vector<int64_t>	msgIds;			///< 表示其他端对收到的消息已经已读了(回执了)
	};
	//////////////////////////////////////////////////////////////////////////

	//! 消息回执变化事件
	struct ReceiptsUpdatedEvent :EventBase
	{
		ConversationId				conId;
		struct MessageReceiptInfo
		{
			int		unreadUserCount = 0;
			int64_t mtime = 0;
		};
		std::map<int64_t, MessageReceiptInfo>	receipts;
	};

	//! 透传命令事件
	struct TransCommandEvent :EventBase
	{
		ConversationId	conId;	///< 所属会话,单聊既是发送方userId,群聊既是群id
		UserId			sender;	///< 发送方
		std::string		cmd;	///< 透传命令
	};

	//! 多端在线设备间透传命令事件
	struct PlatformTransCommandEvent :EventBase
	{
		Platform		platform;	///< 平台
		std::string		cmd;		///< 透传命令
	};



	//! 事件
	using Event = std::variant < NullEvent,
		ForceLogoutEvent,
		RequestAddFriendEvent,
		RejectAddFriendEvent,
		PassAddFriendEvent,
		RemovedByFriendEvent,
		UserUpdatedEvent,

		GroupCreatedEvent,
		GroupDeletedEvent,
		LeavedGroupEvent,
		AddedToGroupEvent,
		RequestJoinGroupEvent,
		RejectJoinGroupEvent,
		AdminRejectJoinGroupEvent,
		RemovedFromGroupEvent,
		GroupMemberSilentChangedEvent,
		GroupInfoUpdatedEvent,
		GroupAdminChangedEvent,
		GroupOwnerChangedEvent,

		NoDisturbUpdatedEvent,
		BlackListUpdatedEvent,
		GroupShieldUpdatedEvent,
		UserInfoUpdatedEvent,
		MessageRetractedEvent,

		MultiFriendAddedEvent,
		MultiFriendRemovedEvent,
		MultiFriendRemarkUpdatedEvent,
		MultiBlackListChangedEvent,
		MultiNoDisturbChangedEvent,
		MultiGroupShieldChangedEvent,

		MultiUnreadMsgCountChangedEvent,
		MultiReceiptChangedEvent,
		ReceiptsUpdatedEvent,

		TransCommandEvent,
		PlatformTransCommandEvent
	> ;

	inline int64_t getEventTime(Event const& ev)
	{
		return std::visit([&](auto&& ev)
		{
			return ev.time;
		}, ev);
	}

	inline int64_t getEventId(Event const& ev)
	{
		return std::visit([&](auto&& ev)
		{
			return ev.eventId;
		}, ev);
	}
}
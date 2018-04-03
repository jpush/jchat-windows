#pragma once

#include <stdint.h>
#include <cstdint>
#include <vector>
#include <tuple>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <utility>
#include "JmcppExport.h"
#include "StrongType.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace Jmcpp
{
	//! 性别
	enum class Gender{ Unknown, Male, Female };

	//! 平台类型
	enum class Platform
	{
		Android = 1,
		iOS/**/ = 2,
		Web/**/ = 3,
		PC/* */ = 4,

		All/**/ = 0xFF
	};

	//! 用户Id
	/**
	* 包含用户名和用户所属的appkey
	**/
	struct UserId
	{
		std::string					username;
		std::string					appKey;

		UserId() = default;

		UserId(std::string const& username, std::string const& appKey = std::string())
			:username(username), appKey(appKey)	{}


		std::string toString() const
		{
			return username + " " + appKey;
		}

		static UserId fromString(std::string const& str)
		{
			UserId userId;
			std::stringstream ss(str);
			ss >> userId.username;
			ss >> userId.appKey;
			return userId;
		}


		bool isValid() const{ return !username.empty(); }

		explicit operator bool() const	{ return isValid(); }

		bool operator<(UserId const& other) const noexcept
		{
			return std::forward_as_tuple(username, appKey) < std::forward_as_tuple(other.username, other.appKey);
		}

		bool operator<=(UserId const& other) const noexcept
		{
			return std::forward_as_tuple(username, appKey) <= std::forward_as_tuple(other.username, other.appKey);
		}

		bool operator>(UserId const& other) const noexcept
		{
			return std::forward_as_tuple(username, appKey) > std::forward_as_tuple(other.username, other.appKey);
		}

		bool operator>=(UserId const& other) const noexcept
		{
			return std::forward_as_tuple(username, appKey) >= std::forward_as_tuple(other.username, other.appKey);
		}

		bool operator==(UserId const& other) const noexcept
		{
			return std::forward_as_tuple(username, appKey) == std::forward_as_tuple(other.username, other.appKey);
		}

		bool operator!=(UserId const& other) const noexcept
		{
			return std::forward_as_tuple(username, appKey) != std::forward_as_tuple(other.username, other.appKey);
		}
	};

	using UserIdList = std::vector<UserId>;

	using GroupId = StrongType::StrongAlias<int64_t, struct GroupIdTag, StrongType::ExplictConvertible, StrongType::EqualityComparable, StrongType::OrderingComparable>;

	using RoomId = StrongType::StrongAlias<int64_t, struct RoomIdTag, StrongType::ExplictConvertible, StrongType::EqualityComparable, StrongType::OrderingComparable>;

	//! 用户信息
	struct UserInfo
	{
		UserId		userId;

		std::string nickname;	//!< 昵称
		std::string avatar;		//!< 头像mediaId

		std::string remark;		//!< 备注
		std::string remarkOther;//!< 备注

		std::string birthday;	//!< 生日
		Gender		gender;		//!< 性别
		std::string signature;	//!< 签名
		std::string region;		//!< 地区
		std::string address;	//!< 地址

		std::string extras;		//!< 自定义json对象字符串
		int64_t		mtime = 0;	//!< 用户信息修改时间戳
	};

	using UserInfoList = std::vector<UserInfo>;


	/**
	* 设置用户信息参数
	* 当字段为空时,表示不设置此信息
	**/
	struct UserInfoParam
	{
		std::optional<std::string>  nickname;	//!< 昵称
		std::optional<std::string>  avatar;		//!< 头像mediaId
		std::optional<std::string>  birthday;	//!< 生日
		std::optional<std::string>  signature;	//!< 签名
		std::optional<Gender> 		gender;		//!< 性别
		std::optional<std::string>  region;		//!< 地区
		std::optional<std::string>  address;	//!< 地址
		std::optional<std::string>  extras;		//!< 自定义json对象字符串
	};

	using  UpdateUserInfoParam = UserInfoParam;

	//! 用户信息,只包含部分信息
	struct UserInfoLite
	{
		UserId		userId;
		std::string nickname;  ///< 昵称
		std::string avatar;	   ///< 头像mediaId
	};
	using UserInfoLiteList = std::vector<UserInfoLite>;


	struct GroupMember
	{
		UserId		userId;
		std::string nickname;	///< 昵称
		std::string avatar;		///< 头像mediaId
		bool		isOwner{};	///< 是否群主
		bool		isSilent{};	///< 是否禁言
		bool		isAdmin{};	///< 是否管理员
	};
	using GroupMemberList = std::vector<GroupMember>;

	//! 群信息
	struct GroupInfo
	{
		GroupId		groupId{};	///< 群Id
		std::string groupName;	///< 群名称
		std::string description;///< 群描述

		std::string avatar;		///< 群头像mediaId
		std::string appKey;		///< 群所属的appkey
		int			maxMemberCount{};///< 最大成员数量
		bool		isPublic{};	///< 公开群：用户可主动申请入群，需群主审核;私有群：只能通过群成员邀请入群，无需审核

		int64_t		ctime = 0;	///< 创建时间
		int64_t		mtime = 0;	///< 修改时间
	};
	using GroupInfoList = std::vector<GroupInfo>;


	struct GetGroupsResult
	{
		size_t			total{};///< 公开群总数
		size_t			start{};///< 分页获取起始索引
		GroupInfoList	groups;	///< 公开群信息
	};


	//! 聊天室信息
	struct RoomInfo
	{
		RoomId		roomId;		///<聊天室名称
		std::string roomName;	///<名称
		std::string description;///<描述

		std::string appKey;		///<所属的appkey
		int			maxMemberCount{};///< 最大聊天室人数
		int			currentMemberCount{};///< 当前聊天室人数

		int64_t		ctime = 0;	///< 创建时间
	};
	using RoomInfoList = std::vector<RoomInfo>;


	struct GetRoomsResult
	{
		size_t			total{};///< 聊天室总数
		size_t			start{};///< 分页获取起始索引
		RoomInfoList	rooms;	///< 聊天室信息
	};

	//! 聊天对象ID,群/用户/聊天室
	class JMCPP_API ChatId
	{
		std::variant<std::monostate, GroupId, RoomId, UserId> _id;
	public:
		ChatId();
		ChatId(GroupId const& groupId);
		ChatId(RoomId const& roomId);
		ChatId(UserId const& userId);

		bool isValid() const;

		bool isGroup() const;
		bool isRoom() const;
		bool isUser() const;

		GroupId getGroupId() const;
		void	setGroupId(GroupId groupId);

		RoomId	getRoomId() const;
		void	setRoomId(RoomId roomId);

		const UserId&	getUserId() const;
		void			setUserId(UserId const& userId);

		void setUsername(std::string const& username);
		void setAppkey(std::string const& appkey);

		bool operator<(ChatId const& other) const;
		bool operator<=(ChatId const& other) const;
		bool operator>(ChatId const& other) const;
		bool operator>=(ChatId const& other) const;
		bool operator==(ChatId const& other) const;
		bool operator!=(ChatId const& other) const;

	};

	using ConversationId = ChatId;

	//! 免打扰信息
	struct NotDisturbInfo
	{
		int					global;	///< 全局免打扰设置: 0:关闭免打扰  1:打开免打扰
		UserInfoLiteList	users;	///< 免打扰的用户
		GroupInfoList		groups; ///< 免打扰的群
	};

	//! 消息回执信息 \sa Client::getMessageReceipts
	struct MessageReceipts
	{
		UserIdList unreadUserList;	//!< 消息未读用户列表
		UserIdList readUserList;	//!< 消息已读用户列表
	};

	//! 多端登录历史记录
	struct MultiLoginRecord
	{
		Platform	platform{};	///< 登录平台
		int64_t		loginTime{};///< 最近登录时间
		bool		online{};	///< 是否在线
		bool		login{};	///< 是否登录
		bool		flag{};		///< 该设备是否被当前登录设备踢出
	};

}


#ifdef _MSC_VER
#pragma warning(pop)
#endif

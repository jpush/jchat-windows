#pragma once
#include <stdint.h>
#include <memory> 
#include <string>
#include <functional>
#include <optional>
#include <pplx/pplxtasks.h>

#include "Version.h"
#include "Configuration.h"
#include "Messages.h"
#include "Event.h"
#include "Exception.h"
#include "JmcppExport.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

/// \defgroup 回调设置
/// \defgroup 注册登录
/// \defgroup 用户信息
/// \defgroup 群组管理
/// \defgroup 聊天室
/// 
/// \defgroup 免打扰管理
/// \defgroup 群屏蔽管理
/// \defgroup 黑名单管理
/// \defgroup 好友相关
/// 
/// \defgroup 消息发送与管理
/// \defgroup 消息回执与未读
/// \defgroup 多媒体资源
/// 
namespace Jmcpp
{
	class ClientImpl;
	/*!
	 * \brief Client类提供了SDK的主要功能
	 */
	class JMCPP_API Client
	{
	public:
		Client(Configuration const& cfg = Configuration(), int = JMCPP_VERSION);

		virtual ~Client();


		//! 设置登录成功回调 \ingroup 回调设置
		/**
		* \ref login 和 \ref relogin 成功后回调
		* \param cb 回调参数 \c firstLogin == true 表示是login成功回调,否则是relogin成功回调,
		**/
		void onLogined(std::function<void(bool firstLogin)> cb);

		//! 设置登录成功回调
		/**
		* \ingroup 回调设置
		* \ref login 和 \ref relogin 成功后回调
		**/
		[[deprecated("this will be removed in a future version")]]
		void onLogined(std::function<void()> cb);

		//! 设置网络非正常断开回调 \ingroup 回调设置
		/**
		* 目前SDK没有在网络断开时自动重连,需要开发者根据需求决定是否重登录,
		* 重新登录应调用\c relogin
		**/
		void onDisconnected(std::function<void()> cb);

		//!设置消息接收回调 \ingroup 回调设置
		void onMessageReceive(std::function<void(MessagePtr)> cb);

		//!设置消息同步回调 \ingroup 回调设置
		void onMessageSync(std::function<void(std::vector<MessagePtr>)> cb);

		//!设置事件通知回调 \ingroup 回调设置
		void onEventReceive(std::function<void(Event)> cb);

		//!设置事件同步回调 \ingroup 回调设置
		void onEventSync(std::function<void(std::vector<Event>)> cb);


		//!登录状态 \ingroup 注册登录
		bool isLogined() const;

		//!网络状态 \ingroup 注册登录
		bool isConnected() const;

		//!当前登录用户Id \ingroup 注册登录
		UserId getCurrentUser() const;



		//!注册 \ingroup 注册登录
		/**
		* \param username 用户名
		* \param password 密码
		* \param auth     鉴权信息
		* \param userInfo 可选用户信息
		*
		**/
		pplx::task<void>	registers(std::string_view username, std::string_view password, Authorization const& auth, UserInfoParam const& userInfo = {}) const;

		//!登录\ingroup 注册登录
		/**
		* \param username 用户名
		* \param password 密码
		* \param auth     鉴权信息,10分钟内有效,过期应重新计算
		* \param encoded 密码是否经过md5编码
		* \param roaming 消息是否漫游,打开消息漫游之后，用户多个设备之间登录时，sdk 会自动同步当前登录用户的历史消息
		*
		* \post isLogined() == true
		* \note 不要在回调里直接调用 \c login, 否则可能会死锁
		**/
		pplx::task<std::vector<MultiLoginRecord>>	login(std::string_view username, std::string_view password, Authorization const& auth, bool encoded = false, int roaming = 1);


		//!断线后重登录 \ingroup 注册登录
		/**
		* 只有在 \ref login 成功后才能调用 \ref relogin
		* \pre  isLogined() == true
		**/
		pplx::task<std::vector<MultiLoginRecord>>	relogin(Authorization const& auth);


		//!退出登录,断开网络连接等,析构自动调用 \ingroup 注册登录
		/**
		* 调用\c logout 后,再次重新登录应该创建一个新的Client
		* \post isLogined() == false
		* \note 阻塞操作,不要在回调里直接调用 \c logout,否则可能会死锁
		**/
		void logout();

		//!获取用户信息
		/**
		* \ingroup 用户信息
		* 获取的用户信息不包含好友备注信息,获取备注信息使用 \c getFriendList
		**/
		pplx::task<UserInfo>	getUserInfo(UserId const& userId) const;


		//!获取当前登录用户的信息
		///\ingroup 用户信息
		pplx::task<UserInfo>	getSelfInfo() const;


		//!设置自己的相关用户信息
		///\ingroup 用户信息
		pplx::task<void>		updateSelfInfo(UpdateUserInfoParam const& userInfo) const;


		//!设置自己的头像 \ingroup 用户信息
		/**
		* \param filePath 图片路径
		* \return 新头像 mediaId
		**/
		pplx::task<std::string>	updateSelfAvatar(std::string_view filePath) const;


		//!设置自己的头像 \ingroup 用户信息
		/**
		* \param data 文件数据指针
		* \param size 数据字节大小
		* \param imageExt 文件扩展名, e.g. imageExt=".jpg"
		* \return 新头像 mediaId
		*
		* 可以使用图片内存数据设置头像,避免先存储为临时文件,再设置图片路径
		**/
		pplx::task<std::string>	updateSelfAvatar(const void* data, size_t size, std::string_view imageExt) const;



		//!设置新密码 \ingroup 用户信息
		pplx::task<void>		updateSelfPassword(std::string_view oldPwd, std::string_view newPwd) const;


		//!获取文件 mediaId 的临时url \ingroup 多媒体资源
		pplx::task<std::string>						getResourceUrl(std::string_view mediaId) const;

		//!获取文件 mediaId 的临时url \ingroup 多媒体资源
		[[deprecated("Use getResourceUrl")]]
		pplx::task<std::string>						getResource(std::string_view mediaId) const;


		//!下载 mediaId 对应的资源,返回字节数组 \ingroup 多媒体资源
		pplx::task<std::vector<unsigned char>>		getResourceData(std::string_view mediaId) const;


		//! 创建群 \ingroup 群组管理
		/**
		* \param groupName		群名称
		* \param description	群描述信息
		* \param filePath		群头像图片路径
		* \param isPublic		公开群:群组群成员入群时是否需要群主审核
		**/
		pplx::task<GroupInfo>		createGroup(std::string_view groupName,
												std::optional<std::string_view> const& description = std::nullopt,
												std::optional<std::string_view> const& filePath = std::nullopt,
												bool isPublic = 0) const;


		//! 解散群		\ingroup 群组管理
		/**
		* 只有群主可以解散群
		**/
		pplx::task<void>			deleteGroup(GroupId groupId) const;


		//! 更新群信息	\ingroup 群组管理
		/**
		* 群信息属性参数 至少一个不是 std::nullopt
		**/
		pplx::task<void>			updateGroupInfo(GroupId	groupId,
													std::optional<std::string_view> const& groupName = std::nullopt,
													std::optional<std::string_view> const& description = std::nullopt,
													std::optional<std::string_view> const& filePath = std::nullopt) const;


		//!退出群 \ingroup 群组管理
		/**
		* \param groupId 群Id
		**/
		[[deprecated("use leaveGroup(GroupId)")]]
		pplx::task<GroupInfo>		exitGroup(GroupId groupId) const;


		//!退出群 \ingroup 群组管理
		/**
		* \param groupId 群Id
		**/
		pplx::task<GroupInfo>		leaveGroup(GroupId groupId) const;


		//!邀请群成员加入群 \ingroup 群组管理
		/**
		* \param groupId 群Id
		* \param userList 要添加的用户Id
		**/
		pplx::task<void>			addGroupMembers(GroupId groupId, UserIdList const& userList) const;


		//!删除群成员 \ingroup 群组管理
		/**
		* \param groupId 群Id
		* \param userList 要删除的用户Id
		**/
		pplx::task<void>			removeGroupMembers(GroupId groupId, UserIdList const& userList) const;


		//! 设置/取消群管理员 \ingroup 群组管理
		/**
		* 只有群主可以设置/取消群管理员
		* \param groupId 群Id
		* \param userList 要设置/取消管理员的群成员
		* \param isAdd true:设置为管理员 false:取消管理员
		**/
		pplx::task<void>			setGroupAdmins(GroupId groupId, UserIdList const& userList, bool isAdd);


		//! 转让群主身份 \ingroup 群组管理
		/**
		* 只有群主可以调用
		* \param groupId 群Id
		* \param user 新群主
		**/
		pplx::task<void>			setGroupOwner(GroupId groupId, UserId user);


		//! 设置群成员禁言 \ingroup 群组管理
		pplx::task<void>			setGroupMemberSilent(GroupId groupId, Jmcpp::UserId const & user, bool isOn)const;


		//!获取用户群列表 \ingroup 群组管理
		pplx::task<GroupInfoList>	getGroups() const;


		//! 获取群信息 \ingroup 群组管理
		pplx::task<GroupInfo>		getGroupInfo(GroupId groupId) const;


		//! 获取群成员列表 \ingroup 群组管理
		pplx::task<GroupMemberList>	getGroupMembers(GroupId groupId) const;




		//! 分页获取appKey下所有公开群		\ingroup 群组管理
		/**
		* \param start 分页起始索引,首页起始索引为0
		**/
		pplx::task<GetGroupsResult>	getAppGroups(size_t start, std::string_view appkey = std::string_view()) const;


		//!申请加入群 \ingroup 群组管理
		/**
		* \param groupId 群id
		* \param requestMsg 入群验证信息
		**/
		pplx::task<void>			joinGroup(GroupId groupId, std::string_view requestMsg = std::string_view()) const;


		//!通过加入群请求 \ingroup 群组管理
		/**
		* \param groupId 群id
		* \param eventId 请求加入群的事件id
		* \param user		请求加入群的用户
		* \param fromUser	邀请者/申请者
		*
		* 如果是主动请求加入群 user == fromUser
		**/
		pplx::task<void>			passJoinGroup(GroupId groupId, int64_t eventId, Jmcpp::UserId const& user, Jmcpp::UserId const& fromUser)const;


		//!拒绝加入群请求 \ingroup 群组管理
		/**
		* \param groupId	群id
		* \param eventId	请求加入群的事件id
		* \param user		请求加入群的用户
		* \param fromUser	邀请者/申请者
		* \param rejectReason 拒绝原因
		*
		* 如果是主动请求加入群 user == fromUser
		**/
		pplx::task<void>			rejectJoinGroup(GroupId groupId, int64_t eventId, Jmcpp::UserId const& user, Jmcpp::UserId const& fromUser, std::string_view rejectReason)const;



		//////////////////////////////////////////////////////////////////////////
		//! 分页获取appkey下的所有聊天室 \ingroup 聊天室
		/**
		* \param start 分页起始索引,首页起始索引为0
		**/
		pplx::task<GetRoomsResult>	getAppRooms(size_t start, std::string_view appkey = std::string_view()) const;


		//! 获取已进入的聊天室 \ingroup 聊天室
		pplx::task<RoomInfoList>	getSelfRooms() const;


		//! 进入聊天室 \ingroup 聊天室
		/**
		* 进入聊天室后,可以收到聊天室的消息
		**/
		pplx::task<void>			enterRoom(RoomId roomId) const;


		//! 离开聊天室 \ingroup 聊天室
		pplx::task<void>			leaveRoom(RoomId roomId) const;


		//! 获取聊天室信息  \ingroup 聊天室
		pplx::task<RoomInfo>		getRoomInfo(RoomId roomId) const;


		//////////////////////////////////////////////////////////////////////////
		//! 获取当前免打扰设置 \ingroup 免打扰管理
		pplx::task<NotDisturbInfo>	getNotDisturb() const;

		//! 设置全局免打扰  \ingroup 免打扰管理
		pplx::task<void>			setGlobalNotDisturb(bool on) const;

		//! 设置用户免打扰 \ingroup 免打扰管理
		pplx::task<void>			setNotDisturb(UserId const& userId, bool on) const;

		//! 设置群免打扰 \ingroup 免打扰管理
		pplx::task<void>			setNotDisturb(GroupId groupId, bool on) const;


		//! 获取群屏蔽列表 \ingroup 群屏蔽管理
		pplx::task<GroupInfoList>	getGroupShields() const;

		//! 设置群屏蔽 \ingroup 群屏蔽管理
		pplx::task<void>			setGroupShield(GroupId groupId, bool on) const;


		//! 获取黑名单列表 \ingroup 黑名单管理
		pplx::task<UserInfoList>	getBlackList() const;

		//! 添加到黑名单 \ingroup 黑名单管理
		pplx::task<void>			addToBlackList(std::vector<UserId> const& usernames) const;

		//! 移出黑名单 \ingroup 黑名单管理
		pplx::task<void>			removeFromBlackList(std::vector<UserId> const& usernames) const;


		//////////////////////////////////////////////////////////////////////////
		//! 请求添加好友  \ingroup 好友相关
		/**
		* 被添加用户收到 \ref RequestAddFriendEvent 事件
		* \sa RequestAddFriendEvent
		**/
		pplx::task<void> addFriend(UserId const& userId, std::string_view requestMsg) const;

		//! 通过添加好友 \ingroup 好友相关
		/**
		* 添加好友请求发起者收到 \ref PassAddFriendEvent 事件
		**/
		pplx::task<void> passAddFriend(UserId const& userId)const;

		//! 拒绝添加好友  \ingroup 好友相关
		/**
		* 添加好友请求发起者收到 \ref RejectAddFriendEvent 事件
		**/
		pplx::task<void> rejectAddFriend(UserId const& userId, std::string_view rejectReason) const;


		//! 删除好友 \ingroup 好友相关
		pplx::task<void>			delFriend(UserId const& userId)const;


		//! 更新好友备注 \ingroup 好友相关
		pplx::task<void>			updateFriendRemark(UserId const& userId, std::string_view remark, std::string_view remarkOthers = {}) const;


		//! 获取好友列表 \ingroup 好友相关
		///
		pplx::task<UserInfoList>	getFriendList()const;


		//////////////////////////////////////////////////////////////////////////
		//! 重置会话未读数,会同步多端登录,调用必成功 \ingroup 消息回执与未读
		pplx::task<void>			clearUnreadCount(ConversationId const& conId)const;


		//! 获取消息的未读和已读用户列表 \ingroup 消息回执与未读
		///
		pplx::task<MessageReceipts>	getMessageReceipts(int64_t msgId)const;


		//! 发送消息回执  \ingroup 消息回执与未读
		/**
		* \param conId	会话Id
		* \param msgIds 已读的消息id
		**/
		pplx::task<void>			sendMessageReceipts(ConversationId const& conId, std::vector<int64_t> const& msgIds)const;


		//////////////////////////////////////////////////////////////////////////

		//! 撤回消息 \ingroup 消息发送与管理
		pplx::task<void>			retractMessage(int64_t msgId)const;


		//! 创建文本消息内容 \ingroup 消息发送与管理
		/**
		* \param text utf8编码的文本消息内容
		**/
		pplx::task<TextContent>		createTextContent(std::string_view text) const;


		//! 从文件创建图片消息内容  \ingroup 消息发送与管理
		/**
		* \param filePath utf8编码图片文件路径,必须包含正确的图片扩展名
		**/
		pplx::task<ImageContent>	createImageContent(std::string_view filePath) const;


		//! 从内存数据创建图片消息内容 \ingroup 消息发送与管理
		/**
		* \param data 数据指针
		* \param size 数据大小
		* \param fileName 自定义文件名,必须包含正确的图片扩展名
		**/
		pplx::task<ImageContent>	createImageContent(const void *data, size_t size, std::string_view fileName) const;


		//! 创建文件消息内容  \ingroup 消息发送与管理
		/**
		* \param filePath utf8编码文件路径
		**/
		pplx::task<FileContent>		createFileContent(std::string_view filePath) const;


		//!创建位置消息内容  \ingroup 消息发送与管理
		/**
		* \param latitude	纬度
		* \param longitude	经度
		* \param scale		地图缩放级别
		* \param label		地点标签
		**/
		pplx::task<LocationContent> createLocationContent(double latitude, double longitude, double scale, std::string_view label) const;


		//! 创建自定义消息内容 \ingroup 消息发送与管理
		/**
		* \param custom utf8编码字符串,包含json对象
		**/
		pplx::task<CustomContent>	createCustomContent(std::string_view custom) const;


		//! 创建一条单聊消息  \ingroup 消息发送与管理
		/**
		* \param targetUserId 发送对象
		* \param content 消息内容
		* \param settings 消息设置
		**/
		MessagePtr					buildMessage(UserId const& targetUserId,
												 MessageContent const& content,
												 MessageSettings const& settings = {}) const;

		//!创建一条群聊消息  \ingroup 消息发送与管理
		/**
		* \param groupId 发送群
		* \param content 消息内容
		* \param settings 消息设置
		* \param atUsers 要at的用户;默认为空,表示不at任何人;UserIdList为空时,at所有人;不空时只at部分人
		**/
		MessagePtr					buildMessage(GroupId groupId,
												 MessageContent const& content,
												 MessageSettings const& settings = {},
												 std::optional<UserIdList> const& atUsers = std::nullopt) const;


		//! 创建一条聊天室消息 \ingroup 消息发送与管理
		/**
		* \param RoomId	聊天室Id
		* \param content 消息内容
		* \param settings 消息设置
		**/
		MessagePtr					buildMessage(RoomId const& roomId,
												 MessageContent const& content,
												 MessageSettings const& settings = {}) const;



		//! 创建一条单聊或群聊或聊天室消息,调用上面的3个函数 \ingroup 消息发送与管理
		MessagePtr					buildMessage(ConversationId const& conId,
												 MessageContent const& content,
												 MessageSettings const& settings = {},
												 std::optional<UserIdList> const& atUsers = std::nullopt) const;

		//!发送消息  \ingroup 消息发送与管理
		/**
		* \param msg 消息对象
		* \return 消息对象,包含有效消息Id
		* \note 同一个消息对象发送多次,SDK认为是同一条消息,目标对象只会收到一次
		**/
		pplx::task<MessagePtr>		sendMessage(MessagePtr const& msg) const;


		//! 发送透传命令  \ingroup 消息发送与管理
		/**
		* 仅单聊和群聊支持透传命令
		* \sa TransCommandEvent
		**/
		pplx::task<void>			sendTransCommand(ConversationId const& conId, std::string_view cmd) const;


		//! 发送透传命令  \ingroup 消息发送与管理
		/**
		* 多端在线设备间透传
		* \sa PlatformTransCommandEvent
		**/
		pplx::task<void>			sendTransCommand(Platform platform, std::string_view cmd) const;



		//!获取消息历史记录 \ingroup 消息发送与管理
		/**
		* \param count 获取消息的数目
		* \param timeAsc 获取的消息按时间升序或降序排序
		* \param msg 如果按时间升序排序,则获取 \c msg 之后 \c count个数目的消息;如果按时间降序排序,则获取\c msg 之前的 \c count个数目的消息;
		*			 如果\c msg为空,则从第一个或最后一个开始获取
		**/
		std::vector<MessagePtr>				getMessages(ConversationId const& conId, std::size_t count, MessagePtr const& msg, bool timeAsc = false) const;


		//!异步获取消息历史记录 \ingroup 消息发送与管理
		pplx::task<std::vector<MessagePtr>> getMessagesAsync(ConversationId const& conId, std::size_t count, MessagePtr const& msg, bool timeAsc = false) const;


		//!获取最新一条消息 \ingroup 消息发送与管理
		MessagePtr							getLatestMessage(ConversationId const& conId) const;


		//! 删除会话消息记录 \ingroup 消息发送与管理
		/**
		* \return 被删除的消息数目
		**/
		int				deleteMessage(ConversationId const& conId, int64_t msgId);


		//! 删除会话所有消息记录 \ingroup 消息发送与管理
		/**
		* \return 被删除的消息数目
		**/
		int				deleteAllMessage(ConversationId const& conId);


		//!获取同步会话列表 \ingroup 消息发送与管理
		pplx::task<std::vector<ConversationId>>		getConversationList() const;

	private:
		Client(Client const&) = delete;
		Client& operator=(Client const&) = delete;
		friend class ClientImpl;
		std::shared_ptr<ClientImpl> _p;
	};

	using ClientPtr = std::shared_ptr<Client>;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include <type_traits>
#include "Types.h"
#include "JmcppExport.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251 4275)
#endif

namespace Jmcpp
{
	struct ContentBase
	{
		std::string	extras;		//!< 消息附属消息 json对象
	};

	struct NullContent :ContentBase
	{
	};

	//! 文本消息内容
	struct TextContent :ContentBase
	{
		std::string text;
	};

	//! 图片消息内容
	struct ImageContent :ContentBase
	{
		std::string mediaId;		//!< 文件mediaId \sa Client::getResourceUrl 
		std::string format;			//!< 图片格式
		int64_t		fileSize = 0;	//!< 图片文件大小,字节
		uint32_t	crc32 = 0;

		int			width = 0;		//!< 图片像素宽
		int			height = 0;		//!< 图片像素高
	};

	//! 文件消息内容
	struct FileContent :ContentBase
	{
		std::string mediaId;		//!< 文件mediaId \sa Client::getResourceUrl  
		std::string format;			//!< 文件格式
		int64_t		fileSize = 0;	//!< 文件大小,字节
		uint32_t	crc32 = 0;

		std::string hash;			//!< 文件hash
		std::string fileName;		//!< 文件名
	};

	//! 位置消息内容
	struct LocationContent :ContentBase
	{
		double				latitude = 0;	//!< 纬度
		double				longitude = 0;	//!< 经度
		double				scale = 1;		//!< 地图缩放级别
		std::string			label;			//!< 地点标签
	};

	//! 位置消息内容
	struct VoiceContent :ContentBase
	{
		std::string mediaId;
		std::string format;
		int64_t		fileSize = 0;
		uint32_t	crc32 = 0;

		std::string hash;
		uint32_t	duration; //!< 语音时长(秒)
	};

	//! 自定义消息内容
	struct CustomContent :ContentBase
	{
		std::string			custom; //!< json object
	};

	//! 消息内容
	using MessageContent = std::variant<
		NullContent,
		TextContent,
		ImageContent,
		FileContent,
		LocationContent,
		VoiceContent,
		CustomContent>;

	//! 消息设置信息
	struct MessageSettings
	{
		bool		needReceipt = true; //!< 消息是否需要回执
		bool		offline = true;		//!< 是否保存离线消息
	};


	class JMCPP_API Message
	{
	public:
		Message();
		virtual ~Message();

		int64_t			msgId = 0;			//!< 消息id
		int64_t			time = 0;			//!< 消息时间戳
		std::string		platform;			//!< 发送消息的设备平台

		bool			isOutgoing = false;	//!< 是否向外消息,i.e.是否是当前登录用户发出的消息
		bool			needReceipt = false;//!< 消息是否需要回执,不需要回执的消息或者已回执的消息为false

		ConversationId	conId;				//!< 消息所属会话Id

		UserId			sender;				//!< 消息发送者
		UserId			receiver;			//!< 如果是单聊,消息接收者

		int				unreadUserCount = 0;		//!< 此消息未读用户数,仅自己发送的消息有效
		int64_t			unreadUserCountMTime = 0;	//!< 此消息未读用户数变化时间

		std::optional<UserIdList>	userList;	//!< 消息@用户列表,群消息有效
		Jmcpp::MessageContent		content;	//!< 消息内容

#if defined(_MSC_VER)
		__declspec(property(get = _getGroupId))
		int64_t groupId[[deprecated]];	//!< 群id, 等于0则不是群消息

		[[deprecated("use conId.getUserId() instead, this will be removed in a future version")]]
		UserId const&	getUser() const{ return conId.getUserId(); };	//!< 如果是单聊,聊天对象Id

		[[deprecated("use conId instead, this will be removed in a future version")]]
		ConversationId 	getConId() const{ return conId; }	//!< 消息所属回话

		[[deprecated("groupId deprecated,use conId.getGroupId() instead, this will be removed in a future version")]]
		int64_t _getGroupId() const{ return conId.getGroupId().get(); }

#endif
	private:
		Message(Message const&) = delete;
		Message& operator=(Message const&) = delete;
		friend class Client;
		friend class ClientImpl;
	};

	using MessagePtr = std::shared_ptr<Message>;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

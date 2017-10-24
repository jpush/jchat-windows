
#include "Util.h"

#include <QxOrm.h>
#include "MetaTypes.h"


QSize JChat::getImageThumbnailSize(QSize const& size)
{
	const double w = 220, h = 300;

	if(size.width() <= 0 || size.height() <= 0)
	{
		return QSize(w, h);
	}

	auto width = qMin<double>(w, size.width());
	auto height = qMin<double>(h, size.height());

	auto r0 = w / h;
	auto r1 = (double)size.width() / size.height();
	if(r0 < r1)
	{
		return QSize(width, width / r1);
	}
	else
	{
		return QSize(height*r1, height);
	}
}

QString JChat::getTimeDisplayString(QDateTime const& dt)
{
	auto now = QDateTime::currentDateTime();
	auto days = dt.daysTo(now);
	if(days < 0)
	{
		return dt.toString("yyyy-MM-dd hh:mm");
	}
	if(days == 0)
	{
		return dt.toString("hh:mm");
	}
	if(days == 1)
	{
		return u8"昨天 ";
	}
	if(days == 2)
	{
		return u8"前天 ";
	}

	static char const* weekDay[] = { u8"", u8"周一 ",u8"周二 ",u8"周三 " ,u8"周四 " ,u8"周五 " ,u8"周六 " ,u8"周日 " };
	if(days > 2 && days < 7)
	{
		return weekDay[dt.date().dayOfWeek()];
	}

	if(dt.date().year() == now.date().year())
	{
		return dt.toString("MM-dd");
	}

	return dt.toString("yyyy-MM-dd");
}

QString JChat::getTimeDisplayStringEx(QDateTime const& dt)
{
	auto now = QDateTime::currentDateTime();
	auto days = dt.daysTo(now);
	if(days < 0)
	{
		return dt.toString("yyyy-MM-dd hh:mm");
	}
	if(days == 0)
	{
		return dt.toString("hh:mm");
	}
	if(days == 1)
	{
		return u8"昨天 " + dt.toString("hh:mm");
	}
	if(days == 2)
	{
		return u8"前天 " + dt.toString("hh:mm");
	}

	static char const* weekDay[] = { u8"", u8"周一 ",u8"周二 ",u8"周三 " ,u8"周四 " ,u8"周五 " ,u8"周六 " ,u8"周日 " };
	if(days > 2 && days < 7)
	{
		return weekDay[dt.date().dayOfWeek()] + dt.toString("hh:mm");
	}

	if(dt.date().year() == now.date().year())
	{
		return dt.toString("MM-dd hh:mm");
	}

	return dt.toString("yyyy-MM-dd");
}

QString JChat::getUserDisplayName(Jmcpp::UserInfo const & userInfo)
{
	if(userInfo.userId)
	{
		if(!userInfo.remark.empty())
		{
			return userInfo.remark.c_str();
		}
		if(!userInfo.nickname.empty())
		{
			return userInfo.nickname.c_str();
		}

		return userInfo.userId.username.c_str();
	}

	return {};
}

QString JChat::getUserDisplayName(Jmcpp::UserId const& userId)
{
	return userId.username.c_str();
}

QString JChat::getUserDisplayName(Jmcpp::GroupMember const& userInfo)
{

	if(!userInfo.nickname.empty())
	{
		return userInfo.nickname.c_str();
	}

	return userInfo.userId.username.c_str();

}

QString JChat::getConversationDisplayName(Jmcpp::ConversationId const& conId)
{
	if(conId.isUser())
	{
		return getUserDisplayName(conId.getUserId());
	}
	else
	{
		return QString::number(conId.getGroupId().get());
	}

	return "?????";
}


QString
JChat::getMessageDisplayString(Jmcpp::MessagePtr const& msg, Jmcpp::UserInfo const& sender)
{
	auto&& content = msg->content;

	if(msg->groupId.get())
	{
		auto name = (msg->isOutgoing ? u8"我" : getUserDisplayName(sender)) + ':';
		if(std::holds_alternative<Jmcpp::TextContent>(content))
		{
			auto cont = std::get<Jmcpp::TextContent>(content);
			return name + cont.text.data();
		}
		else if(std::holds_alternative<Jmcpp::ImageContent>(content))
		{
			return  name + u8"[图片]";
		}
		else if(std::holds_alternative<Jmcpp::FileContent>(content))
		{
			return name + u8"[文件]";
		}
		else if(std::holds_alternative<Jmcpp::LocationContent>(content))
		{
			return  name + u8"[位置]";
		}
		else if(std::holds_alternative<Jmcpp::VoiceContent>(content))
		{
			return  name + u8"[语音]";
		}
		else
		{

		}
	}
	else
	{
		if(std::holds_alternative<Jmcpp::TextContent>(content))
		{
			auto cont = std::get<Jmcpp::TextContent>(content);
			return cont.text.data();
		}
		else if(std::holds_alternative<Jmcpp::ImageContent>(content))
		{
			return u8"[图片]";
		}
		else if(std::holds_alternative<Jmcpp::FileContent>(content))
		{
			return u8"[文件]";
		}
		else if(std::holds_alternative<Jmcpp::LocationContent>(content))
		{
			return u8"[位置]";

		}
		else if(std::holds_alternative<Jmcpp::VoiceContent>(content))
		{
			return u8"[语音]";
		}
		else
		{

		}
	}

	return {};
}


#pragma once


#include <QString>
#include <QDateTime>
#include <QSize>
#include <Jmcpp/Client.h>

namespace JChat
{

	QSize	getImageThumbnailSize(QSize const& size);

	QString getTimeDisplayString(QDateTime const& dt);

	QString getTimeDisplayStringEx(QDateTime const& dt);


	QString getUserDisplayName(Jmcpp::UserId const& userId);

	QString getUserDisplayName(Jmcpp::GroupMember const& userInfo);

	QString getUserDisplayName(Jmcpp::UserInfo const& userInfo);

	QString getConversationDisplayName(Jmcpp::ConversationId const& conId);

	QString getMessageDisplayString(Jmcpp::MessagePtr const& msg, Jmcpp::UserInfo const& sender = {});

}


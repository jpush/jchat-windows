
#include "ClientObject.h"

#include <QtNetwork/QtNetwork>

#include "Settings.h"
#include "Util.h"


QDir JChat::ClientObject::_storageRootPath = []{
	QDir dir = "./JChat";
	dir.makeAbsolute();
	dir.mkdir("./");
	return dir;
}();

namespace
{
	QString userAvatarFile()
	{
		return u8":/image/resource/头像60px.png";
	}

	QString groupAvatarFile()
	{
		return u8":/image/resource/群组头像60px.png";
	}

	QPixmap const& userAvatar()
	{
		static QPixmap img{ u8":/image/resource/头像60px.png" };
		return img;
	}

	QPixmap const& groupAvatar()
	{
		static QPixmap img{ u8":/image/resource/群组头像60px.png" };
		return img;
	}
}


Jmcpp::Configuration
JChat::ClientObject::getSDKConfig()
{
	Settings setting(QCoreApplication::applicationFilePath() + ".ini");

	Jmcpp::Configuration cfg;
	cfg.serverUrl = setting.value<QString>("serverUrl", QString()).toStdString();
	cfg.uploadUrl = setting.value<QString>("uploadUrl", QString()).toStdString();
	cfg.downloadUrl = setting.value<QString>("downloadUrl", QString()).toStdString();

	cfg.logLevel = 3;
	return cfg;
}

Jmcpp::Authorization
JChat::ClientObject::getAuthorization()
{
	Settings setting(QCoreApplication::applicationFilePath() + ".ini");
	auto appKey = setting.value<QString>("appKey", QString());
	auto masterSecret = setting.value<QString>("masterSecret", QString());
	if(appKey.isEmpty() || (appKey == "4f7aef34fb361292c566a1cd" && masterSecret.isEmpty()))
	{
		Jmcpp::Authorization auth;
		auth.appKey = "4f7aef34fb361292c566a1cd";
		auth.randomStr = "022cd9fd995849b58b3ef0e943421ed9";
		auth.timestamp = "1507793640389";
		auth.signature = "c5e83666337c32e9bcf6e948edf606f0";
		return auth;
	}
	else
	{
		auto time = QDateTime::currentDateTime().toMSecsSinceEpoch();
		auto str = QString("appkey=%1&timestamp=%2&random_str=022cd9fd995849b58b3ef0e943421ed9&key=%3")
			.arg(appKey).arg(time).arg(masterSecret);

		Jmcpp::Authorization auth;
		auth.appKey = appKey.toStdString();
		auth.randomStr = "022cd9fd995849b58b3ef0e943421ed9";
		auth.timestamp = QString::number(time).toStdString();
		auth.signature = QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5).toHex().toStdString();
		return auth;
	}

}


//////////////////////////////////////////////////////////////////////////
JChat::ClientObject::ClientObject(Jmcpp::Configuration const& cfg) :Client(cfg)
{
	(void)(this | qTrack);

	userAvatar();
	groupAvatar();

	qApp->setProperty(staticMetaObject.className(), QVariant::fromValue(this));

	_netThread = new QThread();
	_manager = new QNetworkAccessManager();
	_manager->moveToThread(_netThread);

	connect(_netThread, &QThread::finished, [=]{_manager->deleteLater(); });

	_netThread->start();

	init();
}

JChat::ClientObject::~ClientObject()
{
	_netThread->quit();
	_netThread->wait();

	delete _netThread;

	auto name = _memoryDb.connectionName();
	_memoryDb.close();
	QSqlDatabase::removeDatabase(name);
}


pplx::task<void>
JChat::ClientObject::getAllData()
{
	auto tasks = {
		updateFriendList(),
		updateGroupList(),
		getNotDisturbInfo(),
		getGroupShieldsInfo(),
		updateBlackList()
	};
	return pplx::when_all(begin(tasks), end(tasks));
}

pplx::task<QString>
JChat::ClientObject::getUserAvatarFilePath(Jmcpp::UserId userId, std::string avatarMediaId)
{
	auto self = this | qTrack;
	if(avatarMediaId.empty())
	{
		co_return userAvatarFile();
	}
	auto id = QString::fromStdString(avatarMediaId);
	auto fileName = id.section('/', -1);
	auto filePath = _avatarCachePath.absoluteFilePath(fileName);

	if(auto suffix = QFileInfo(filePath).suffix(); !suffix.isEmpty())
	{
		filePath.resize(filePath.size() - suffix.size() - 1);
	}

	filePath += ".jpg";


	if(QFile::exists(filePath))
	{
		co_return filePath;
	}

	auto data = co_await this->getResourceData(avatarMediaId);

	QImage img;
	img.loadFromData(data);
	img = img.scaledToWidth(60, Qt::TransformationMode::SmoothTransformation);
	img.save(filePath);
	co_await self;
	co_return filePath;
}

pplx::task<Jmcpp::UserInfo>
JChat::ClientObject::getCacheUserInfo(Jmcpp::UserId userId, bool update)
{
	if(!update)
	{
		std::unique_lock<std::mutex> locker(_lock);
		if(auto iter = _friendInfos.find(userId); iter != _friendInfos.end())
		{
			auto info = iter->second;
			locker.unlock();
			co_return info;
		}

		if(auto iter = _userInfos.find(userId); iter != _userInfos.end())
		{
			auto info = iter->second;
			locker.unlock();
			co_return info;
		}
	}


	auto self = shared_from_this();
	UserInfo info = co_await getUserInfo(userId);
	{
		std::unique_lock<std::mutex> locker(_lock);
		if(auto iter = _friendInfos.find(userId); iter != _friendInfos.end())
		{
			info.remark = iter->second.remark;
			info.remarkOther = iter->second.remarkOther;

			iter->second = info;
		}

		_userInfos.insert_or_assign(userId, info);
	}
	co_return info;
}

pplx::task<Jmcpp::GroupInfo>
JChat::ClientObject::getCacheGroupInfo(int64_t groupId, bool update)
{
	auto self = shared_from_this();
	if(!update)
	{
		std::unique_lock<std::mutex> locker(_lock);
		if(auto iter = _groupInfos.find(groupId); iter != _groupInfos.end())
		{
			auto info = iter->second;
			locker.unlock();

			if(info.groupName.empty())
			{
				auto dummyName = co_await _getGroupDummyName(info.groupId);
				info.groupName = dummyName.toStdString();
				locker.lock();
				if(auto it = _groupInfos.find(info.groupId); it != _groupInfos.end())
				{
					it->second.groupName = dummyName.toStdString();
				}
			}

			co_return info;
		}
	}

	auto info = co_await getGroupInfo(groupId);
	if(info.groupName.empty())
	{
		auto dummyName = co_await _getGroupDummyName(info.groupId);
		info.groupName = dummyName.toStdString();
	}

	{
		std::unique_lock<std::mutex> locker(_lock);
		_groupInfos.insert_or_assign(groupId, info);
	}
	co_return info;
}

pplx::task<QPixmap>
JChat::ClientObject::getCacheUserAvatar(Jmcpp::UserId userId, std::string avatarMediaId, bool update)
{
	auto self = this | qTrack;

	auto userIdStr = QString::fromStdString(userId.toString());
	auto avatarFilePath = _avatarCachePath.absoluteFilePath(userIdStr + ".jpg");
	auto key = userIdStr + "avatar";
	if(!update)
	{
		co_await self;
		auto pixmap = QPixmap(avatarFilePath);
		if(!pixmap.isNull())
		{
			//pixmap = pixmap.scaledToWidth(60, Qt::TransformationMode::SmoothTransformation);
			co_return pixmap;
		}
	}

	auto avatar = avatarMediaId;
	if(avatar.empty())
	{
		UserInfo info = co_await getCacheUserInfo(userId, update);
		avatar = info.avatar;
		if(avatar.empty())
		{
			co_return userAvatar();
		}
	}

	auto data = co_await this->getResourceData(avatar);

	QImage img;
	img.loadFromData(data);
	img = img.scaledToWidth(60, Qt::TransformationMode::SmoothTransformation);
	img.save(avatarFilePath);

	co_await self;

	co_return QPixmap::fromImage(img);
}

pplx::task<QPixmap>
JChat::ClientObject::getCacheGroupAvatar(int64_t groupId, std::string avatarMediaId /*= std::string()*/, bool update /*= false*/)
{
	auto self = this | qTrack;
	auto idStr = QString::number(groupId);
	auto avatarFilePath = _avatarCachePath.absoluteFilePath(idStr + ".jpg");
	auto key = idStr + "avatar";
	if(!update)
	{
		co_await self;
		auto pixmap = QPixmap(avatarFilePath);
		if(!pixmap.isNull())
		{
			pixmap = pixmap.scaledToWidth(60, Qt::TransformationMode::SmoothTransformation);
			co_return pixmap;
		}
	}

	auto avatar = avatarMediaId;
	if(avatar.empty())
	{
		auto info = co_await getCacheGroupInfo(groupId, update);
		avatar = info.avatar;
		if(avatar.empty())
		{
			co_return groupAvatar();
		}
	}

	auto data = co_await this->getResourceData(avatar);

	co_await self;
	QPixmap pixmap;
	pixmap.loadFromData(data);
	pixmap = pixmap.scaledToWidth(60, Qt::TransformationMode::SmoothTransformation);
	pixmap.save(avatarFilePath);
	co_return pixmap;
}

pplx::task<QPixmap>
JChat::ClientObject::getCacheAvatar(Jmcpp::ConversationId const& conId, std::string avatarMediaId /*= std::string()*/, bool update /*= false*/)
{
	Q_ASSERT(conId.isValid());
	if(conId.isGroup())
	{
		return getCacheGroupAvatar(conId.getGroupId(), avatarMediaId, update);
	}
	else if(conId.isUser())
	{
		return getCacheUserAvatar(conId.getUserId(), avatarMediaId, update);
	}
	return {};
}

pplx::task<QString>
JChat::ClientObject::getUserDisplayName(Jmcpp::UserId userId, bool update)
{
	auto info = co_await getCacheUserInfo(userId, update);
	co_return JChat::getUserDisplayName(info);
}

pplx::task<QString>
JChat::ClientObject::getGroupDisplayName(int64_t groupId, bool update)
{
	auto info = co_await getCacheGroupInfo(groupId, update);

	co_return info.groupName.c_str();
}

pplx::task<QString>
JChat::ClientObject::getDisplayName(Jmcpp::ConversationId const& conId, bool update)
{
	Q_ASSERT(conId.isValid());
	if(conId.isGroup())
	{
		return getGroupDisplayName(conId.getGroupId(), update);
	}
	else if(conId.isUser())
	{
		return getUserDisplayName(conId.getUserId(), update);
	}
	return {};
}

QPixmap
JChat::ClientObject::getConversationImage(Jmcpp::ConversationId const& conId)
{
	if(conId.isUser())
	{
		auto userId = conId.getUserId();
		auto userIdStr = QString::fromStdString(userId.toString());
		auto avatarFilePath = _avatarCachePath.absoluteFilePath(userIdStr + ".jpg");
		if(QFile::exists(avatarFilePath))
		{
			return QPixmap{ avatarFilePath };
		}
		return userAvatar();
	}
	else
	{
		auto groupId = conId.getGroupId();
		auto idStr = QString::number(groupId);
		auto avatarFilePath = _avatarCachePath.absoluteFilePath(idStr + ".jpg");
		if(QFile::exists(avatarFilePath))
		{
			return QPixmap{ avatarFilePath };
		}
		return groupAvatar();
	}
}

pplx::task<QPixmap>
JChat::ClientObject::getStaticMap(Jmcpp::LocationContent const& content)
{
	auto self = this | qTrack;

	auto name = QString("%1,%2.jpg").arg(content.longitude, 0, 'f', 6).arg(content.latitude, 0, 'f', 6);
	auto filePath = imageFilePath(name);

	if(QFile::exists(filePath))
	{
		co_await self;
		auto pixmap = QPixmap(filePath);
		co_return pixmap;
	}
	else
	{
		QString url = "http://api.map.baidu.com/staticimage/v2?ak=ZG9nF68Mgrk0V9Eyb3efhNtT7FxREG2X&center=%1,%2&width=300&height=200&zoom=15&markers=%3|%1,%2&markerStyles=m,,0xff0000";
		url = url.arg(content.longitude).arg(content.latitude).arg(content.label.c_str());

		auto data = co_await _download(url);

		QImage img;
		img.loadFromData(data);
		img.save(filePath);
		co_await self;
		co_return QPixmap::fromImage(img);
	}
}

QString
JChat::ClientObject::imageFilePath(Jmcpp::ImageContent const& content) const
{
	auto id = QString::fromStdString(content.mediaId);
	auto fileName = id.section('/', -1);
	auto filePath = _imageReceivedPath.absoluteFilePath(fileName);

	if(QFileInfo(filePath).suffix().isEmpty())
	{
		if(!content.format.empty())
		{
			filePath += '.';
			filePath += content.format.c_str();
		}
		else
		{
			filePath += ".png";
		}
	}

	return filePath;
}

QString
JChat::ClientObject::imageFilePath(QString const& fileName) const
{
	return _imageReceivedPath.absoluteFilePath(fileName);
}

QString
JChat::ClientObject::filePath(Jmcpp::FileContent const& content) const
{
	auto fileName = QString::fromStdString(content.fileName);

	return _fileReceivedPath.absoluteFilePath(fileName);
}


pplx::task<QString>
JChat::ClientObject::getImage(Jmcpp::ImageContent const& content)
{
	auto filePath = imageFilePath(content);
	if(QFile::exists(filePath))
	{
		co_return filePath;
	}
	else
	{
		auto data = co_await this->getResourceData(content.mediaId);
		co_await ResumeBackground{};
		QImage image;
		image.loadFromData(data);
		image.save(filePath);
		co_return filePath;
	}
}



pplx::task<QString>
JChat::ClientObject::getFile(Jmcpp::FileContent const& content, std::function<void(int)> progress)
{
	auto filePath = this->filePath(content);

	if(QFile::exists(filePath))
	{
		co_return filePath;
	}
	else
	{
		auto data = co_await this->getResourceData(content.mediaId, progress);
		QFile file(filePath);
		if(file.open(QFile::ReadWrite | QFile::Truncate))
		{
			file.write((const char*)data.data(), data.size());
			file.close();
		}
		co_return filePath;
	}
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
bool
JChat::ClientObject::isFriend(Jmcpp::UserId const& userId)
{
	std::unique_lock<std::mutex> locker(_lock);
	auto iter = _friendInfos.find(userId);
	return iter != _friendInfos.end();
}

bool
JChat::ClientObject::isNotDisturb(int64_t groupId)
{
	std::unique_lock<std::mutex> locker(_lock);
	auto iter = _notDisturbGroup.find(groupId);
	return iter != _notDisturbGroup.end();
}

bool
JChat::ClientObject::isNotDisturb(Jmcpp::UserId const& userId)
{
	std::unique_lock<std::mutex> locker(_lock);
	auto iter = _notDisturbUser.find(userId);
	return iter != _notDisturbUser.end();
}

bool JChat::ClientObject::isNotDisturb(Jmcpp::ConversationId const& conId)
{
	if(conId.isGroup())
	{
		isNotDisturb(conId.getGroupId());
	}
	else if(conId.isUser())
	{
		isNotDisturb(conId.getUserId());
	}
	return false;
}


bool
JChat::ClientObject::isShield(int64_t groupId)
{
	std::unique_lock<std::mutex> locker(_lock);
	auto iter = _shieldGroup.find(groupId);
	return iter != _shieldGroup.end();
}

bool
JChat::ClientObject::isInBlackList(Jmcpp::UserId const& userId)
{
	std::unique_lock<std::mutex> locker(_lock);

	return _blackList.find(userId) != _blackList.end();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

pplx::task<QByteArray>
JChat::ClientObject::getResourceData(std::string const& mediaId, std::function<void(int)> progress)
{
	std::unique_lock<std::mutex> locker(_lock);
	auto iter = _resourceTask.find(mediaId);
	if(iter != _resourceTask.end())
	{
		return iter->second;
	}

	auto url = Client::getResource(mediaId);
	auto task = url.then([this, self = shared_from_this(), progress = std::move(progress)](pplx::task<std::string> urlTask)
	{
		auto url = urlTask.get();
		return _download(QUrl(QString::fromStdString(url)), std::move(progress));
	});

	_resourceTask.insert_or_assign(mediaId, task);

	task.then([this, self = shared_from_this(), mediaId](pplx::task<QByteArray> t)
	{
		std::unique_lock<std::mutex> locker(_lock);
		_resourceTask.erase(mediaId);
		try
		{
			t.wait();
		}
		catch(...)
		{
		}
	});
	return task;
}


pplx::task<void>
JChat::ClientObject::updateFriendRemark(Jmcpp::UserId const& userId, std::string const& remark, std::string const& remarkOthers)
{
	try
	{
		auto self = shared_from_this();
		co_await Client::updateFriendRemark(userId, remark, remarkOthers);

		std::unique_lock<std::mutex> locker(_lock);
		auto iter = _friendInfos.find(userId);
		if(iter != _friendInfos.end())
		{
			iter->second.remark = remark;
			iter->second.remarkOther = remarkOthers;
		}
	}
	catch(...)
	{
		throw;
	}
}


pplx::task<void>
JChat::ClientObject::setGroupShield(int64_t groupId, bool on)
{
	auto self = shared_from_this();
	co_await Client::setGroupShield(groupId, on);
	std::unique_lock<std::mutex> locker(_lock);
	if(on)
	{
		_shieldGroup.insert(groupId);
	}
	else
	{
		_shieldGroup.erase(groupId);
	}

	Q_EMIT groupShieldChanged(groupId, on);
}

pplx::task<void>
JChat::ClientObject::setNotDisturb(int64_t groupId, bool on)
{
	auto self = shared_from_this();
	co_await Client::setNotDisturb(groupId, on);
	{
		std::unique_lock<std::mutex> locker(_lock);
		if(on)
		{
			_notDisturbGroup.insert(groupId);
		}
		else
		{
			_notDisturbGroup.erase(groupId);
		}
	}
	Q_EMIT notDisturbChanged(groupId, on);
}

pplx::task<void>
JChat::ClientObject::addToBlackList(std::vector<Jmcpp::UserId>  usernames)
{
	auto self = shared_from_this();

	co_await Client::addToBlackList(usernames);

	std::unique_lock<std::mutex> locker(_lock);
	_blackList.insert(usernames.begin(), usernames.end());
}

pplx::task<void>
JChat::ClientObject::removeFromBlackList(std::vector<Jmcpp::UserId>  usernames)
{
	auto self = shared_from_this();
	co_await Client::removeFromBlackList(usernames);

	std::unique_lock<std::mutex> locker(_lock);
	for(auto&& userId : usernames)
	{
		_blackList.erase(userId);
	}
}

pplx::task<void>
JChat::ClientObject::setNotDisturb(Jmcpp::UserId  userId, bool on)
{
	auto self = shared_from_this();
	co_await Client::setNotDisturb(userId, on);
	{
		std::unique_lock<std::mutex> locker(_lock);
		if(on)
		{
			_notDisturbUser.insert(userId);
		}
		else
		{
			_notDisturbUser.erase(userId);
		}
	}
	Q_EMIT notDisturbChanged(userId, on);
}

pplx::task<Jmcpp::MessagePtr>
JChat::ClientObject::sendMessage(Jmcpp::MessagePtr const& msg) const
{
	auto result = Client::sendMessage(msg);

	Q_EMIT messageSent(msg);

	return result;
}


pplx::task<void>
JChat::ClientObject::addFriend(Jmcpp::UserId userId, std::string const& requestMsg)
{
	auto self = shared_from_this();
	try
	{

		co_await Client::addFriend(userId, requestMsg);
		Q_EMIT requestAddFriendSent(userId, false);
	}
	catch(Jmcpp::ServerException& e)
	{
		if(e.code() == 881302)
		{	//已经是好友
			Q_EMIT requestAddFriendSent(userId, true);

			updateFriendList();
		}
		else
		{
			throw;
		}
	}
}

pplx::task<void>
JChat::ClientObject::passAddFriend(Jmcpp::UserId const& userId)
{
	auto self = shared_from_this();
	co_await Client::passAddFriend(userId);
	updateFriendList();
}


pplx::task<QByteArray>
JChat::ClientObject::_download(QUrl const& url, std::function<void(int)> progress)
{
	pplx::task_completion_event<QByteArray> tce;
	post(_netThread, [this, self = shared_from_this(), url, tce, progress = std::move(progress)]
	{
		auto reply = _manager->get(QNetworkRequest(url));
		if(progress)
		{
			connect(reply, &QNetworkReply::downloadProgress, reply,
					[progress = std::move(progress),flag = false](qint64 bytesReceived, qint64 bytesTotal) mutable
			{
				if(bytesTotal == bytesReceived && bytesTotal == 0)
				{
					return;
				}

				if(bytesTotal == -1)
				{
					if(!flag)
					{
						post(qApp, [=]
						{
							progress(-1);
						});
						flag = true;
					}
				}
				else
				{
					post(qApp, [=]
					{
						progress((float)bytesReceived / bytesTotal * 100);
					});
				}
			});
		}


		connect(reply, &QNetworkReply::finished, reply, [=]
		{
			reply->deleteLater();
			auto data = reply->readAll();
			tce.set(data);
		});

		connect(reply, (void(QNetworkReply::*)(QNetworkReply::NetworkError)) &QNetworkReply::error, reply, [=](QNetworkReply::NetworkError err)
		{
			reply->deleteLater();
			if(err != QNetworkReply::NetworkError::NoError)
			{
				tce.set_exception(std::system_error(std::make_error_code(std::errc::io_error)));
			}
		});

		connect(reply, &QNetworkReply::sslErrors, reply, [=](const QList<QSslError> &errors)
		{
			reply->deleteLater();
			tce.set_exception(std::system_error(std::make_error_code(std::errc::protocol_not_supported)));
		});
	});

	return pplx::create_task(tce);

}

/************************************************************************/
/*                                                                      */
/************************************************************************/
pplx::task<void>
JChat::ClientObject::updateFriendList()
{
	try
	{
		auto self = shared_from_this();
		auto friends = co_await getFriendList();

		{
			std::unique_lock<std::mutex> locker(_lock);
			_friendInfos.clear();
			for(auto&& info : friends)
			{
				_friendInfos.insert_or_assign(info.userId, info);
			}
		}

		Q_EMIT friendListUpdated(friends);
	}
	catch(std::runtime_error& e)
	{
	}
}

pplx::task<void>
JChat::ClientObject::updateGroupList()
{
	try
	{
		auto self = shared_from_this();
		auto groups = co_await getGroups();

		{
			for(auto&& info : groups)
			{
				if(info.groupName.empty())
				{
					auto dymmyName = co_await _getGroupDummyName(info.groupId);
					info.groupName = dymmyName.toStdString();
				}
			}

			std::unique_lock<std::mutex> locker(_lock);
			_groupInfos.clear();
			for(auto&& info : groups)
			{
				_groupInfos.insert_or_assign(info.groupId, info);
			}
		}

		Q_EMIT groupListUpdated(groups);
	}
	catch(std::runtime_error& e)
	{
	}
}


pplx::task<void>
JChat::ClientObject::getNotDisturbInfo()
{
	auto self = shared_from_this();
	auto info = co_await getNotDisturb();

	std::unique_lock<std::mutex> locker(_lock);
	globalNotDisturb = info.global;

	_notDisturbUser.clear();
	for(auto&& user : info.users)
	{
		_notDisturbUser.insert(user.userId);
	}

	_notDisturbGroup.clear();
	for(auto&& group : info.groups)
	{
		_notDisturbGroup.insert(group.groupId);
	}
}

pplx::task<void>
JChat::ClientObject::getGroupShieldsInfo()
{
	auto info = co_await getGroupShields();
	std::unique_lock<std::mutex> locker(_lock);
	_shieldGroup.clear();
	for(auto&& group : info)
	{
		_shieldGroup.insert(group.groupId);
	}
}


pplx::task<void>
JChat::ClientObject::updateBlackList()
{
	auto self = shared_from_this();
	auto blacklist = co_await getBlackList();

	std::unique_lock<std::mutex> locker(_lock);
	_blackList.clear();

	for(auto&& info : blacklist)
	{
		_blackList.emplace(info.userId);
	}
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
void
JChat::ClientObject::init()
{

	onLogined([=](){

		{
			initPath();
			initDB();
			getAllData().then([this, self = shared_from_this()](pplx::task<void> t)
			{
				try
				{
					t.get();
				}
				catch(std::runtime_error& e)
				{
					qWarning() << e.what();
				}
				Q_EMIT userLogined();
			});
		}
	});

	onDisconnected([=]()
	{
		Q_EMIT disconnected();
	});

	onMessageReceive([=](Jmcpp::MessagePtr msgPtr)
	{
		Q_EMIT messageReceived(msgPtr);
	});

	onMessageSync([=](std::vector<Jmcpp::MessagePtr> msgs)
	{
		Q_EMIT messagesReceived(msgs);
	});

	onEventReceive([this](Jmcpp::Event event){

		std::visit([=](auto&& ev)
		{
			onEvent(ev);
		}, event);
	});

	onEventSync([this](std::vector<Jmcpp::Event> eventList)
	{
		for(auto&& event : eventList)
		{
			std::visit([=](auto&& ev)
			{
				onEvent(ev);
			}, event);
		}
	});

}
//////////////////////////////////////////////////////////////////////////

void
JChat::ClientObject::onEvent(Jmcpp::ReceiptsUpdatedEvent const& e)
{
	Q_EMIT receiptsUpdatedEvent(e);
}

void JChat::ClientObject::onEvent(Jmcpp::MultiGroupShieldChangedEvent const& e)
{
	std::unique_lock<std::mutex> locker(_lock);
	if(e.added)
	{
		_shieldGroup.insert(e.groupId);
	}
	else
	{
		_shieldGroup.erase(e.groupId);
	}

	emit groupShieldChanged(e.groupId, e.added);
}

void JChat::ClientObject::onEvent(Jmcpp::MultiNoDisturbChangedEvent const& e)
{
	std::unique_lock<std::mutex> locker(_lock);

	if(e.conId.isUser())
	{
		if(e.added)
		{
			_notDisturbUser.insert(e.conId.getUserId());
		}
		else
		{
			_notDisturbUser.erase(e.conId.getUserId());
		}
	}
	else if(e.conId.isUser())
	{
		if(e.added)
		{
			_notDisturbGroup.insert(e.conId.getGroupId());
		}
		else
		{
			_notDisturbGroup.erase(e.conId.getGroupId());
		}
	}

	Q_EMIT notDisturbChanged(e.conId, e.added);
}

void
JChat::ClientObject::onEvent(Jmcpp::MultiFriendRemarkUpdatedEvent const& e)
{
	std::unique_lock<std::mutex> locker(_lock);
	if(auto iter = _friendInfos.find(e.user); iter != _friendInfos.end())
	{
		iter->second.remark = e.remark;
		iter->second.remarkOther = e.remarkOther;
	}
	else
	{
		//Q_ASSERT(false);
		qWarning();

		updateFriendList();
	}


	Q_EMIT userInfoUpdated(e.user);

	emit multiFriendRemarkUpdatedEvent(e);
}

None
JChat::ClientObject::onEvent(Jmcpp::UserInfoUpdatedEvent const& e)
{
	auto self = shared_from_this();

	auto userId = e.user;

	auto info = co_await getCacheUserInfo(userId, true);

	co_await getCacheUserAvatar(userId, info.avatar, true);

	if(e.user == getCurrentUser())
	{
		Q_EMIT selfInfoUpdated(userId);
	}
	else
	{
		Q_EMIT userInfoUpdated(userId);
	}
}

void
JChat::ClientObject::onEvent(Jmcpp::ForceLogoutEvent const& e)
{
	Q_EMIT forceLogoutEvent(e);
}

void
JChat::ClientObject::onEvent(Jmcpp::MessageRetractedEvent const& e)
{
	Q_EMIT messageRetracted(e);
}

void
JChat::ClientObject::onEvent(Jmcpp::RequestAddFriendEvent const& e)
{
	Q_EMIT requestAddFriendEvent(e);
}

void
JChat::ClientObject::onEvent(Jmcpp::PassAddFriendEvent const& e)
{
	Q_EMIT passAddFriendEvent(e);

	//#TODO 优化
	updateFriendList();
}

void
JChat::ClientObject::onEvent(Jmcpp::RejectAddFriendEvent const& e)
{
	Q_EMIT rejectAddFriendEvent(e);
}

void
JChat::ClientObject::onEvent(Jmcpp::RemovedByFriendEvent const& e)
{
	updateFriendList();
}

void
JChat::ClientObject::onEvent(Jmcpp::UserUpdatedEvent const& e)
{
	qDebug() << e.description.c_str();
}

void
JChat::ClientObject::onEvent(Jmcpp::GroupCreatedEvent const& e)
{
	Q_EMIT groupCreatedEvent(e);
}

void
JChat::ClientObject::onEvent(Jmcpp::LeavedGroupEvent const& e)
{
	Q_EMIT leavedGroupEvent(e);
}

void
JChat::ClientObject::onEvent(Jmcpp::AddedToGroupEvent const& e)
{
	Q_EMIT addedToGroupEvent(e);
}

void
JChat::ClientObject::onEvent(Jmcpp::RemovedFromGroupEvent const& e)
{
	Q_EMIT removedFromGroupEvent(e);
}

None
JChat::ClientObject::onEvent(Jmcpp::GroupInfoUpdatedEvent const& e)
{
	auto groupId = e.groupId;
	auto ev = e;

	auto info = co_await this->getCacheGroupInfo(groupId, true);
	co_await this->getCacheGroupAvatar(groupId, info.avatar, true);

	Q_EMIT groupInfoUpdated(groupId);

	Q_EMIT groupInfoUpdatedEvent(ev);
}

void
JChat::ClientObject::onEvent(Jmcpp::MultiUnreadMsgCountChangedEvent const& e)
{
	Q_EMIT multiUnreadMsgCountChangedEvent(e);
}


//////////////////////////////////////////////////////////////////////////
void
JChat::ClientObject::initPath()
{
	auto root = _storageRootPath;

	_userRootPath = root.absoluteFilePath(getCurrentUser().toString().data());
	_userRootPath.mkpath("./");

	_avatarCachePath = _userRootPath.absoluteFilePath("avatar");
	_avatarCachePath.mkpath("./");


	_fileReceivedPath = _userRootPath.absoluteFilePath("file");
	_fileReceivedPath.mkpath("./");

	_imageReceivedPath = _userRootPath.absoluteFilePath("image");
	_imageReceivedPath.mkpath("./");
}

void
JChat::ClientObject::initDB()
{
	std::unique_lock<std::mutex> locker(_lock);

	Q_ASSERT(this->getCurrentUser());

	QDir dir = _userRootPath;
	auto path = dir.absoluteFilePath("user.db");

	qx::QxSqlDatabase::clearAllDatabases();

	qx::QxSqlDatabase::getSingleton()->setDriverName("QSQLITE");
	qx::QxSqlDatabase::getSingleton()->setDatabaseName(path);
	qx::QxSqlDatabase::getSingleton()->setHostName("localhost");
	qx::QxSqlDatabase::getSingleton()->setUserName("root");
	qx::QxSqlDatabase::getSingleton()->setPassword("");

	qx::QxSqlDatabase::getSingleton()->setConnectOptions("QSQLITE_BUSY_TIMEOUT=30000");


	QSqlDatabase db = qx::QxSqlDatabase::getSingleton()->getDatabase();

	initDB(db);
	//
	if(!_memoryDb.isOpen())
	{
		_memoryDb = QSqlDatabase::addDatabase("QSQLITE", "memory");
		_memoryDb.setDatabaseName(":memory:");
		_memoryDb.open();
		initDB(_memoryDb);
	}
}

pplx::task<QString>
JChat::ClientObject::_getGroupDummyName(int64_t groupId)
{
	auto self = shared_from_this();
	auto members = co_await getGroupMembers(groupId);
	QStringList userNames;
	try
	{
		for(auto&& member : members)
		{
			auto info = co_await getCacheUserInfo(member.userId);
			userNames.push_back(JChat::getUserDisplayName(info));
			if(userNames.size() > 4)
			{
				break;
			}
		}
	}
	catch(std::runtime_error& e)
	{

	}
	co_return userNames.join(',');
}



static void
createTable(qx::IxClass* pClass, QString const& tableName, QSqlDatabase const& db)
{
	QSqlQuery query(db);
	QString sql;
	QTextStream ts(&sql);

	auto table = tableName.isEmpty() ? pClass->getName() : tableName;

	ts << "CREATE TABLE IF NOT EXISTS " << table << " ( " << endl;

	bool hasPrimaryKey = false;
	qx::IxDataMemberX * pDataMemberX = pClass->getDataMemberX();
	for(long l = 0, count = pDataMemberX->count_WithDaoStrategy(); (pDataMemberX && l < count); l++)
	{
		qx::IxDataMember * p = pDataMemberX->get_WithDaoStrategy(l);
		if(!p) { continue; }

		if(l != 0)
		{
			ts << " ," << endl;
		}

		auto c = p->getNameCount();
		for(auto i = 0; i < c; i++)
		{
			if(i != 0)
			{
				ts << " ," << endl;
			}

			ts << p->getName(i) << " " << p->getSqlType(i);

			if(p->getNotNull()) // NOT NULL
			{
				ts << " NOT NULL";
			}

			auto defValue = p->getDefaultValue();

			if(defValue.isValid())
			{
				ts << " DEFAULT " << defValue.toString();
			}

			if(p->getIsPrimaryKey() && p->getAutoIncrement() && !hasPrimaryKey) // AUTO INCREMENT
			{
				ts << " PRIMARY KEY AUTOINCREMENT";
				hasPrimaryKey = true;
			}

			if(p->getIsUnique())
			{
				ts << " UNIQUE ";
			}
		}
	}

	for(long l = 0, count = pDataMemberX->count_WithDaoStrategy();
		(pDataMemberX && l < count); l++)
	{
		qx::IxDataMember * p = pDataMemberX->get_WithDaoStrategy(l);
		if(!p) { continue; }

		if(p->getIsPrimaryKey() && !hasPrimaryKey)
		{
			ts << " ," << endl;
			ts << "PRIMARY KEY( " << p->getName().replace('|', ',') << " )";
			break;
		}
	}

	ts << "\n)" << endl;

	for(long l = 0, count = pDataMemberX->count_WithDaoStrategy();
		(pDataMemberX && l < count); l++)
	{
		qx::IxDataMember * p = pDataMemberX->get_WithDaoStrategy(l);
		if(!p) { continue; }

		if(p->getIsIndex())
		{
			if(p->getIsUnique())
			{
				ts << "CREATE UNIQUE INDEX IF NOT EXISTS ";
			}
			else
			{
				ts << "CREATE INDEX IF NOT EXISTS ";
			}

			ts << p->getName() << "_idx " << "ON" << table
				<< " (" << p->getName() << " )" << endl;
		}
	}

	if(!query.exec(sql))
	{
		if(auto sqlError = query.lastError(); sqlError.isValid())
		{
			qDebug() << sqlError.databaseText();
			qDebug() << sqlError.driverText();
			qDebug() << sqlError.number();
			qDebug() << sqlError.type();
		}
	}
}


void
JChat::ClientObject::initDB(QSqlDatabase const& db)
{
	qx::QxSession session(db, true);

	// Fetch all C++ persistents classes registered in QxOrm context
	qx::QxCollection<QString, qx::IxClass *> * pAllClasses = qx::QxClassX::getAllClasses();
	if(!pAllClasses) { qAssert(false); return; }

	// Fetch all tables into database (this is a Qt function)
	QStringList tables = db.tables();

	for(long k = 0; k < pAllClasses->count(); k++)
	{
		qx::IxClass * pClass = pAllClasses->getByIndex(k);
		if(!pClass) { continue; }

		pClass->getAllValidator();//

		if(pClass->isKindOf("qx::service::IxParameter") || pClass->isKindOf("qx::service::IxService")) { continue; }

		if(tables.contains(pClass->getName()))
		{
			continue;
		}

		createTable(pClass, QString(), db);
	}


}


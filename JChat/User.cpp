#include "User.h"

#include "ClientObject.h"
#include "Util.h"
#include "scope.hpp"

JChat::UserHandle::UserHandle(std::shared_ptr<ClientObject> const& co, Jmcpp::UserId const& userId)
	:_userId(userId)
	, _co(co)
{
	_userInfo.userId = userId;
}

bool JChat::UserHandle::isValid() const
{
	return _isValid;
}

void
JChat::UserHandle::attachUser(User* user)
{
	std::unique_lock<std::shared_mutex> locker(_mutex);
	_users.insert(user);
}

void
JChat::UserHandle::detachUser(User* user)
{
	std::unique_lock<std::shared_mutex> locker(_mutex);
	_users.erase(user);
}

None
JChat::UserHandle::_updateInfo(bool force)
{
	if(_updating > 0)
	{
		co_return;
	}

	if(_isValid && !force)
	{
		co_return;
	}

	auto self = shared_from_this();
	auto co = _co.lock();
	if(!co)
	{
		co_return;
	}


	Jmcpp::UserInfo info;
	Jmcpp::UserInfo oldInfo;

	decltype(_users) users;

	{
		++_updating;
		SCOPE_EXIT(){ --_updating; };

		info = co_await co->getUserInfo(_userId);

		std::unique_lock<std::shared_mutex> locker(_mutex);
		if(_userInfo.mtime >= info.mtime)
		{
			co_return;
		}

		oldInfo = _userInfo;
		_userInfo = info;
		_isValid = true;
		users = _users;
	}


	QString filePath;
	if(info.avatar != oldInfo.avatar)
	{
		try
		{
			filePath = co_await co->getUserAvatarFilePath(_userId, info.avatar);
			std::unique_lock<std::shared_mutex> locker(_mutex);
			_avatarFilePath = filePath;
		}
		catch(std::runtime_error& e)
		{

		}
	}

	co_await ResumeMainThread{};

	for(auto&& user : users)
	{
		if(user)
		{
			if(info.nickname != oldInfo.nickname)
				Q_EMIT user->nickNameChanged(_userInfo.nickname.c_str());

			if(info.remark != oldInfo.remark)
				Q_EMIT user->remarkChanged(_userInfo.remark.c_str());

			if(info.nickname != oldInfo.nickname || info.remark != oldInfo.remark)
				Q_EMIT user->displayNameChanged(getUserDisplayName(_userInfo));

			if(info.avatar != oldInfo.avatar)
			{
				Q_EMIT user->avatarMediaIdChanged(_userInfo.avatar.c_str());

				Q_EMIT user->avatarFilePathChanged(filePath);
			}

			if(info.birthday != oldInfo.birthday)
				Q_EMIT user->birthdayChanged(_userInfo.birthday.c_str());

			if(info.gender != oldInfo.gender)
				Q_EMIT user->genderChanged((int)_userInfo.gender);

			if(info.signature != oldInfo.signature)
				Q_EMIT user->signatureChanged(_userInfo.signature.c_str());

			if(info.region != oldInfo.region)
				Q_EMIT user->regionChanged(_userInfo.region.c_str());

			if(info.address != oldInfo.address)
				Q_EMIT user->addressChanged(_userInfo.address.c_str());

		}
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

JChat::User::User(std::shared_ptr<UserHandle> const& handle, QObject* parent)
	:QObject(parent)
	, _handle(handle)
{
	_handle->attachUser(this);
}

JChat::User::~User()
{
	_handle->detachUser(this);
}

bool JChat::User::isValid() const
{
	return _handle->_isValid;
}

std::optional<Jmcpp::UserInfo>
JChat::User::getUserInfo() const
{
	if(!_handle->_isValid)
	{
		_handle->_updateInfo(false);
		return {};
	}

	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return _handle->_userInfo;
}

QString
JChat::User::getUserName() const
{
	//std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return QString::fromStdString(_handle->_userId.username);
}

QString
JChat::User::getNickName() const
{
	_handle->_updateInfo(false);
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return QString::fromStdString(_handle->_userInfo.nickname);
}

QString
JChat::User::getRemark() const
{
	_handle->_updateInfo(false);
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return QString::fromStdString(_handle->_userInfo.remark);
}

QString
JChat::User::getDisplayName() const
{
	_handle->_updateInfo(false);
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return getUserDisplayName(_handle->_userInfo);
}



QString JChat::User::getAvatarFilePath() const
{
	_handle->_updateInfo(false);
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return _handle->_avatarFilePath;
}

QString JChat::User::getAvatarMediaId() const
{
	_handle->_updateInfo(false);
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return _handle->_userInfo.avatar.c_str();
}

QString
JChat::User::getBirthDay() const
{
	_handle->_updateInfo(false);
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return _handle->_userInfo.birthday.c_str();
}

int
JChat::User::getGender() const
{
	_handle->_updateInfo(false);
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return (int)_handle->_userInfo.gender;
}

QString
JChat::User::getSignature() const
{
	_handle->_updateInfo(false);
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return _handle->_userInfo.signature.c_str();
}

QString
JChat::User::getRegion() const
{
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return _handle->_userInfo.region.c_str();
}

QString
JChat::User::getAddress() const
{
	_handle->_updateInfo(false);
	std::shared_lock<std::shared_mutex> locker(_handle->_mutex);
	return _handle->_userInfo.address.c_str();
}

void
JChat::User::update(bool force /*= false*/)
{
	_handle->_updateInfo(force);
}


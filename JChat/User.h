#pragma once

#include <set>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include <QObject>
#include <QPointer>
#include <Jmcpp/Client.h>

#include "await.h"
namespace JChat {

	class ClientObject;

	class User;

	class UserHandle :public std::enable_shared_from_this<UserHandle>
	{
		friend class ClientObject;
		friend class User;
	public:
		UserHandle(std::shared_ptr<ClientObject> const& co, Jmcpp::UserId const& userId);

		bool isValid() const;
	private:
		void attachUser(User* user);

		void detachUser(User* user);

		None _updateInfo(bool force = false);

	private:
		std::weak_ptr<ClientObject>	_co;

		std::atomic<bool>			_isValid = false;
		std::atomic<int>			_updating = 0;

		std::shared_mutex			_mutex;
		std::set< QPointer<User> >	_users;

		Jmcpp::UserId	_userId;

		Jmcpp::UserInfo _userInfo;
		QString			_avatarFilePath;
	};

	class User : public QObject
	{
		Q_OBJECT

			Q_PROPERTY(bool isValid READ isValid NOTIFY validChanged)

			Q_PROPERTY(QString userName READ getUserName)
			Q_PROPERTY(QString nickName READ getNickName NOTIFY nickNameChanged)

			Q_PROPERTY(QString remark READ getRemark NOTIFY remarkChanged)

			Q_PROPERTY(QString displayName READ getDisplayName NOTIFY displayNameChanged)

			Q_PROPERTY(QString avatarMediaId READ getAvatarMediaId NOTIFY avatarMediaIdChanged)

			Q_PROPERTY(QString avatarFilePath READ getAvatarFilePath NOTIFY avatarFilePathChanged)

			Q_PROPERTY(QString birthday READ getBirthDay NOTIFY birthdayChanged)
			Q_PROPERTY(int gender READ getGender NOTIFY genderChanged)
			Q_PROPERTY(QString signature READ getSignature NOTIFY signatureChanged)
			Q_PROPERTY(QString region READ getRegion NOTIFY regionChanged)
			Q_PROPERTY(QString address READ getAddress NOTIFY addressChanged)

	public:
		~User();

	public:
		bool isValid() const;

		std::optional<Jmcpp::UserInfo> getUserInfo() const;

		QString getUserName() const;
		QString getNickName() const;

		QString getRemark() const;

		QString getDisplayName() const;

		QString	getAvatarFilePath() const;

		QString	getAvatarMediaId() const;

		QString getBirthDay() const;

		int		getGender() const;

		QString getSignature() const;

		QString getRegion() const;

		QString getAddress() const;


		Q_SLOT void update(bool force = false);

	Q_SIGNALS:
		void validChanged(bool valid);

		void nickNameChanged(QString const&);

		void remarkChanged(QString const&);

		void displayNameChanged(QString const&);

		void avatarFilePathChanged(QString const&);

		void avatarMediaIdChanged(QString const&);

		void birthdayChanged(QString const&);

		void genderChanged(int);

		void signatureChanged(QString const&);

		void regionChanged(QString const&);

		void addressChanged(QString const&);

	protected:
		User(std::shared_ptr<UserHandle> const& handle, QObject* parent = 0);

	private:
		std::shared_ptr<UserHandle> _handle;

		friend class UserHandle;
		friend class ClientObject;

	};

} // namespace JChat

#pragma once

#include "ClientObject.h"
#
namespace JChat {

	class RememberedAccount
	{
	public:
		RememberedAccount();
		~RememberedAccount();

		QList<Account>		getRememberedUsers();

		void				addRememberedUsers(Account const& accout);

		void				removeRememberedUsers(Account const& accout);

		void				clearUsers();
	private:
		QSqlDatabase db;
	};

} // namespace JChat

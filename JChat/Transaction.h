#pragma once

#include <QObject>
#include <QSqlDatabase>
namespace JChat {

	class Transaction
	{
		QSqlDatabase _db;
		bool		_transaction = false;
		bool		_shouldExecute = true;
	public:
		Transaction();

		Transaction(QSqlDatabase const& db);

		~Transaction();

		bool commit();

		bool rollback();
	private:

		bool beginTransaction() const;
	};

} // namespace JChat

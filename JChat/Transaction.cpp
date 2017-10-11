#include "Transaction.h"

#include <QSqlQuery>
#include <QSqlError>

#include <QxOrm.h>

JChat::Transaction::Transaction() :_db(qx::QxSqlDatabase::getDatabase())
{
	_transaction = beginTransaction();
}


JChat::Transaction::Transaction(QSqlDatabase const& db) : _db(db)
{
	_transaction = beginTransaction();
}

JChat::Transaction::~Transaction()
{
	if(_shouldExecute)
	{
		if(_db.lastError().isValid())
		{
			rollback();
		}
		else
		{
			commit();
		}
	}
}

bool JChat::Transaction::commit()
{
	if(!_transaction)
	{
		return false;
	}
	auto ret = _db.commit();
	_shouldExecute = false;
	return ret;
}

bool JChat::Transaction::rollback()
{
	if(!_transaction)
	{
		return false;
	}
	bool ret = _db.rollback();
	_shouldExecute = false;
	return ret;
}


bool JChat::Transaction::beginTransaction() const
{
	if(!_db.isOpen() || _db.isOpenError())
		return false;

	QSqlQuery q(_db);
	if(!q.exec(QLatin1String("BEGIN IMMEDIATE TRANSACTION")))
	{
		qDebug() << "Transaction::Unable to begin transaction";
		return false;
	}

	return true;
}

#include "RememberedAccount.h"

#include <QDir>
#include <QDateTime>


JChat::RememberedAccount::RememberedAccount()
{
	auto rootPath = ClientObject::storageRootPath();

	auto path = rootPath.absoluteFilePath("all.db");

	db = QSqlDatabase::addDatabase("QSQLITE", "root");
	db.setDatabaseName(path);
	db.setHostName("localhost");
	db.setUserName("root");
	db.setPassword("");
	db.open();
	if(!db.tables().contains("Account"))
	{
		qx::dao::create_table<Account>(&db);
	}
}

JChat::RememberedAccount::~RememberedAccount()
{
	db.close();
}

QList<JChat::Account>
JChat::RememberedAccount::getRememberedUsers()
{
	qx_query query;
	query.orderDesc("lastTime");

	QList<Account> accounts;
	qx::dao::fetch_by_query(query, accounts, &db);
	return accounts;
}

void
JChat::RememberedAccount::addRememberedUsers(Account const& accout)
{
	auto tmp = accout;
	tmp.lastTime = QDateTime::currentDateTime();
	qx::dao::save(tmp, &db);
}

void JChat::RememberedAccount::removeRememberedUsers(Account const& accout)
{
	auto tmp = accout;
	qx::dao::delete_by_id(tmp, &db);
}

void JChat::RememberedAccount::clearUsers()
{
	qx::dao::delete_all<Account>(&db);
}

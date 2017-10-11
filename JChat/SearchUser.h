#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include "ui_SearchUser.h"

#include "ClientObject.h"

namespace JChat {

	class SearchUser : public QWidget
	{
		Q_OBJECT

	public:

		SearchUser(QWidget *parent = Q_NULLPTR, bool multiChecked = true);

		~SearchUser();


		pplx::task<void> searchUser(ClientObjectPtr co, QString const& text);


		QStandardItemModel* model() const { return _model; }

		Q_SIGNAL void itemChecked(QStandardItem * item, bool checked);

	private:
		Ui::SearchUser ui;


		QStandardItemModel* _model;
	};

} // namespace JChat

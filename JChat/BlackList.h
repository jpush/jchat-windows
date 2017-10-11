#pragma once

#include <QWidget>
#include <QStandardItemModel>

#include "ClientObject.h"


#include "ui_BlackList.h"

namespace JChat
{
	class BlackList : public QWidget
	{
		Q_OBJECT

	public:
		BlackList(ClientObjectPtr const& co, QWidget *parent = Q_NULLPTR);
		~BlackList();


		pplx::task<void> getBlackList();


	protected:

		void onCloseClicked(QStandardItem* item);

	private:
		Ui::BlackList ui;

		ClientObjectPtr _co;
		QStandardItemModel* _model;
	};
}


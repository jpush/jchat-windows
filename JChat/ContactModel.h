#pragma once


#include <QPainter>
#include <QTextItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>


#include "ClientObject.h"

namespace JChat{

	class ClientObject;

	class ContactModel : public QStandardItemModel
	{
		Q_OBJECT

	public:
		enum Role
		{
			ConIdRole= Qt::UserRole + 1,
			ImageRole ,
			TitleRole,
			InfoRole,
		};

		ContactModel(ClientObjectPtr const& co, QObject* parent = nullptr);

		~ContactModel();

		QStandardItem* getFriendEventRootItem()
		{
			return this->item(0);
		}

		QStandardItem* getFriendRootItem()
		{
			return this->item(1);
		}

		QStandardItem* getGroupRootItem()
		{
			return this->item(2);
		}


		void onUserInfoUpdated(Jmcpp::UserId const& userId);


		None			_updateItemUserInfo(QStandardItem* item, Jmcpp::UserId userId);

		None			_updateItemGroupInfo(QStandardItem* item, Jmcpp::GroupId groupId);

	private:
		ClientObjectPtr _co;
	};


	class ContactDelegate :public QStyledItemDelegate
	{

	public:
		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;


		virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	};

}
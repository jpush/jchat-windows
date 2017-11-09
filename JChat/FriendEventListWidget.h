#pragma once

#include <QListWidget>
#include <QStyledItemDelegate>

#include "ui_FriendEventListWidget.h"

#include "ClientObject.h"

namespace JChat {

	class FriendEventItemWidget;

	class FriendEventListWidget : public QListWidget
	{
		Q_OBJECT

	public:
		enum Role
		{
			EventId = Qt::UserRole + 1
		};

		FriendEventListWidget(QWidget *parent = Q_NULLPTR);
		~FriendEventListWidget();

		void setClientObject(ClientObjectPtr const& co);

		ClientObjectPtr const& getClientObject() const{ return _co; }


		int				unreadCount() const;

		void			clearUnread();

		Q_SIGNAL void	unreadChanged(int count);
	protected:

		Q_SLOT None onPassClicked(Jmcpp::UserId const&, int64_t);
		Q_SLOT None onRejectClicked(Jmcpp::UserId const&, int64_t);


		void updateItem(FriendEventT const&);

		void loadFromDB();

		FriendEventItemWidget* createItemWidget();

	private:
		Ui::FriendEventListWidget ui;

		ClientObjectPtr _co;
	};


	class UnreadCountDelegate :public QStyledItemDelegate
	{

	public:
		using QStyledItemDelegate::QStyledItemDelegate;

		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	};


} // namespace JChat

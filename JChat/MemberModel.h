#pragma once

#include <QObject>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include "ClientObject.h"

namespace JChat {

	class MemberModel :public QStandardItemModel
	{
		Q_OBJECT
	public:
		enum Role
		{
			UserIdRole= Qt::UserRole + 1,
			AvatarIdRole,
			NameInfo,
			ImageRole,
			IsOwnerRole,
		};

		MemberModel(ClientObjectPtr const& co, int64_t groupId, QObject *parent = nullptr);
		~MemberModel();

		QStandardItemModel* getCompleterModel() const{ return _completerModel; }


		None updateItems();

		bool isOwner() const
		{
			return _owner && _owner == _currentUser;
		}

		void onUserInfoUpdated(Jmcpp::UserId const& userId)
		{
			
		}


		void onLeavedGroupEvent(Jmcpp::LeavedGroupEvent const& e);

		void onRemovedFromGroupEvent(Jmcpp::RemovedFromGroupEvent const& e);

		None onAddedToGroupEvent(Jmcpp::AddedToGroupEvent const&);

		Q_SIGNAL void removeMemberClicked(Jmcpp::UserId const&);

		Q_SIGNAL void memberInfoClicked(Jmcpp::UserId const&);

	private:

		std::optional<Jmcpp::UserId> _owner;
		std::optional<Jmcpp::UserId> _currentUser;

		int64_t _groupId = 0;

		ClientObjectPtr		_co;
		QStandardItemModel* _completerModel = nullptr;

		friend class MemberDelegate;
	};


	class MemberDelegate :public QStyledItemDelegate
	{

	public:
		using  QStyledItemDelegate::QStyledItemDelegate;

		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;


		virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

	};

} // namespace JChat

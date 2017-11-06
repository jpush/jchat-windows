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
			UserIdRole = Qt::UserRole + 1,
			AvatarIdRole,
			NameInfo,
			ImageRole,
			IsOwnerRole,
			IsSlientRole
		};

		MemberModel(ClientObjectPtr const& co, Jmcpp::GroupId groupId, QObject *parent = nullptr);
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

		Jmcpp::GroupId _groupId = 0;

		ClientObjectPtr		_co;
		QStandardItemModel* _completerModel = nullptr;

		friend class MemberDelegate;
	};



} // namespace JChat

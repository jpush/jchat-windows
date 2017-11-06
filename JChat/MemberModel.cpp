#include "MemberModel.h"

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

#include "Util.h"
#include "ProxyModel.h"
#include "ModelRange.h"
namespace JChat {

	MemberModel::MemberModel(ClientObjectPtr const& co, Jmcpp::GroupId groupId, QObject *parent)
		: QStandardItemModel(parent)
		, _co(co)
		, _groupId(groupId)
	{
		_completerModel = new QStandardItemModel(this);


		connect(co.get(), &ClientObject::userInfoUpdated, this, &MemberModel::onUserInfoUpdated);

		connect(_co.get(), &ClientObject::addedToGroupEvent, this, &MemberModel::onAddedToGroupEvent);

		connect(_co.get(), &ClientObject::leavedGroupEvent, this, &MemberModel::onLeavedGroupEvent);

		connect(_co.get(), &ClientObject::removedFromGroupEvent, this, &MemberModel::onRemovedFromGroupEvent);
	}

	MemberModel::~MemberModel()
	{
	}

	None MemberModel::updateItems()
	{
		auto co = _co;
		auto self = this | qTrack;
		auto groupId = _groupId;

		auto currentUser = co->getCurrentUser();
		_currentUser = currentUser;

		auto members = co_await co->getGroupMembers(groupId);


		co_await self;
		this->clear();

		{
			_completerModel->clear();
			auto item = new QStandardItem();
			item->setText(u8"ËùÓÐÈË");
			item->setIcon(QIcon(":/image/resource/logo.ico"));
			_completerModel->appendRow(item);
		}


		QStandardItem* owner = nullptr;
		for(auto&& info : members)
		{
			if(info.isOwner)
			{
				_owner = info.userId;
			}

			auto name = getUserDisplayName(info);
			{
				auto item = new QStandardItem();
				item->setData(QVariant::fromValue(info.userId), UserIdRole);
				item->setData(QVariant::fromValue(info.avatar), AvatarIdRole);
				item->setData(getUserDisplayName(info), NameInfo);
				//item->setData(QVariant::fromValue(image), ImageRole);
				this->appendRow(item);
				if(info.isOwner)
				{
					item->setData(true, IsOwnerRole);
					owner = item;
				}
			}

			{
				if(info.userId != currentUser)
				{
					auto item = new QStandardItem();
					item->setText(name);
					item->setData(QVariant::fromValue(info.userId), UserIdRole);
					item->setData(QVariant::fromValue(info.avatar), AvatarIdRole);
					_completerModel->appendRow(item);
				}
			}
		}

		if(owner)
		{
			this->insertRow(0, this->takeRow(owner->row()));
		}

		std::vector<QPersistentModelIndex> items;
		std::vector<QPersistentModelIndex> completerItems;

		for(auto&& index : this | depthFirst)
		{
			items.emplace_back(index.current->index());
		}

		for(auto&& index : _completerModel | depthFirst)
		{
			if(index.current->row() != 0)
			{
				completerItems.emplace_back(index.current->index());
			}
		}

		auto info = co_await co->getCacheGroupInfo(groupId);

		//////////////////////////////////////////////////////////////////////////
		for(auto&& index : items)
		{
			if(!index.isValid()){ continue; }
			auto item = itemFromIndex(index);

			auto userId = item->data(UserIdRole).value<Jmcpp::UserId>();
			auto avatarId = item->data(AvatarIdRole).value<std::string>();

			auto name = co_await co->getUserDisplayName(userId);
			auto image = co_await co->getCacheUserAvatar(userId, avatarId);
			co_await self;

			if(!index.isValid()){ continue; }
			item = itemFromIndex(index);

			item->setData(name, NameInfo);
			item->setData(QVariant::fromValue(image), ImageRole);
		}

		

		co_await self;

		for(auto&& index : completerItems)
		{
			if(!index.isValid()){ continue; }
			auto item = _completerModel->itemFromIndex(index);

			auto userId = item->data(UserIdRole).value<Jmcpp::UserId>();
			auto avatarId = item->data(AvatarIdRole).value<std::string>();

			auto name = co_await co->getUserDisplayName(userId);
			auto image = co_await co->getCacheUserAvatar(userId, avatarId);
			co_await self;

			if(!index.isValid()){ continue; }
			item = _completerModel->itemFromIndex(index);

			item->setData(name, NameInfo);
			item->setData(QVariant::fromValue(image), ImageRole);
		}

	}


	None MemberModel::onAddedToGroupEvent(Jmcpp::AddedToGroupEvent const& e)
	{
		if(e.groupId != _groupId)
		{
			co_return;
		}
		auto co = _co;
		auto self = this | qTrack;

		std::vector<QPersistentModelIndex> items;
		std::vector<QPersistentModelIndex> completerItems;

		for(auto&& userId : e.users)
		{
			auto item = new QStandardItem();
			item->setData(QVariant::fromValue(userId), UserIdRole);
			item->setData(getUserDisplayName(userId), NameInfo);
			this->appendRow(item);

			items.emplace_back(item->index());

			{
				auto item = new QStandardItem();
				item->setText(getUserDisplayName(userId));
				item->setData(QVariant::fromValue(userId), UserIdRole);
				_completerModel->appendRow(item);

				completerItems.emplace_back(item->index());
			}
		}


		//////////////////////////////////////////////////////////////////////////
		for(auto&& index : items)
		{
			if(!index.isValid()){ continue; }
			auto item = itemFromIndex(index);

			auto userId = item->data(UserIdRole).value<Jmcpp::UserId>();
			auto avatarId = item->data(AvatarIdRole).value<std::string>();

			auto name = co_await co->getUserDisplayName(userId);
			auto image = co_await co->getCacheUserAvatar(userId, avatarId);
			co_await self;

			if(!index.isValid()){ continue; }
			item = itemFromIndex(index);

			item->setData(name, NameInfo);
			item->setData(QVariant::fromValue(image), ImageRole);
		}

		co_await self;

		for(auto&& index : completerItems)
		{
			if(!index.isValid()){ continue; }
			auto item = _completerModel->itemFromIndex(index);

			auto userId = item->data(UserIdRole).value<Jmcpp::UserId>();
			auto avatarId = item->data(AvatarIdRole).value<std::string>();

			auto name = co_await co->getUserDisplayName(userId);
			auto image = co_await co->getCacheUserAvatar(userId, avatarId);
			co_await self;

			if(!index.isValid()){ continue; }
			item = _completerModel->itemFromIndex(index);

			item->setData(name, NameInfo);
			item->setData(QVariant::fromValue(image), ImageRole);
		}

		co_return;
	}

	void MemberModel::onLeavedGroupEvent(Jmcpp::LeavedGroupEvent const& e)
	{
		if(e.groupId != _groupId)
		{
			return;
		}
		for(auto&& item : this | depthFirst)
		{
			auto userId = item->data(UserIdRole).value<Jmcpp::UserId>();
			if(userId == e.fromUser)
			{
				removeRow(item->row());
				break;
			}
		}

		for(auto&& item : _completerModel | depthFirst)
		{
			auto userId = item->data(UserIdRole).value<Jmcpp::UserId>();
			if(userId == e.fromUser)
			{
				_completerModel->removeRow(item->row());
				break;
			}
		}
	}

	void MemberModel::onRemovedFromGroupEvent(Jmcpp::RemovedFromGroupEvent const& e)
	{
		if(e.groupId != _groupId)
		{
			return;
		}
		std::vector<QPersistentModelIndex> indexs;
		for(auto&& item : this | depthFirst)
		{
			auto userId = item->data(UserIdRole).value<Jmcpp::UserId>();

			if(auto iter = std::find(e.users.begin(), e.users.end(), userId); iter != e.users.end())
			{
				indexs.emplace_back(item->index());
			}
		}
		for(auto&& index : indexs)
		{
			if(index.isValid())
			{
				removeRow(index.row());
			}
		}
		indexs.clear();
		for(auto&& item : _completerModel | depthFirst)
		{
			auto userId = item->data(UserIdRole).value<Jmcpp::UserId>();
			if(auto iter = std::find(e.users.begin(), e.users.end(), userId); iter != e.users.end())
			{
				indexs.emplace_back(item->index());
			}
		}
		for(auto&& index : indexs)
		{
			if(index.isValid())
			{
				_completerModel->removeRow(index.row());
			}
		}
	}

} // namespace JChat

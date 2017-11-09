
#include "ConversationModel.h"
#include "ClientObject.h"
#include "Util.h"
#include "ModelRange.h"

namespace JChat{

	ConversationModel::ConversationModel(ClientObject* co, QObject* parent /*= 0*/)
		:QStandardItemModel(parent)
		, _co(co)
	{
		qApp->setProperty(staticMetaObject.className(), QVariant::fromValue(this));

		connect(_co, &ClientObject::userLogined, this, &ConversationModel::onUserLogined);


		//connect(_co, &ClientObject::groupListUpdated, this, [=](Jmcpp::GroupInfoList const & groups)
		//{
		//	std::vector<QPersistentModelIndex> toRemove;
		//	for(auto&& item : this | depthFirst)
		//	{
		//		if(auto conId = item->data(Role::ConIdRole).value<Jmcpp::ConversationId>(); conId.isGroup())
		//		{
		//			if(std::find_if(groups.begin(), groups.end(), [&](auto&& g){ return conId == g.groupId; }) == end(groups))
		//			{
		//				toRemove.emplace_back(item->index());
		//			}
		//		}
		//	}

		//	for(auto&& index : toRemove)
		//	{
		//		if(index.isValid())
		//		{
		//			this->removeRow(index.row());
		//		}
		//	}
		//});

		connect(_co, &ClientObject::userInfoUpdated, this, &ConversationModel::onUserInfoUpdated);
		connect(_co, &ClientObject::groupInfoUpdated, this, &ConversationModel::onGroupInfoUpdated);

		connect(_co, &ClientObject::notDisturbChanged, this, [=](Jmcpp::ConversationId const& conId, bool on)
		{
			for(auto&& item : this | depthFirst)
			{
				if(item->data(Role::ConIdRole).value<Jmcpp::ConversationId>() == conId)
				{
					item->setData(on, Role::NotDisturb);
				}
			}
		});

		connect(_co, &ClientObject::groupShieldChanged, this, [=](Jmcpp::GroupId groupId, bool on)
		{
			for(auto&& item : this | depthFirst)
			{
				if(item->data(Role::ConIdRole).value<Jmcpp::ConversationId>() == groupId)
				{
					item->setData(on, Role::ShieldRole);
				}
			}
		});

		connect(_co, &ClientObject::messageSent, this, [=](Jmcpp::MessagePtr const& msg)
		{
			addConversationItem(msg);
		});

		connect(_co, &ClientObject::messageReceived, this, [=](Jmcpp::MessagePtr const& msg)
		{
			addConversationItem(msg);
		});
		connect(_co, &ClientObject::messagesReceived, this, [=](std::vector<Jmcpp::MessagePtr> const& msgs)
		{
			auto&& msg = msgs.back();
			addConversationItem(msg);
		});
	}

	void ConversationModel::addConversationItem(Jmcpp::MessagePtr const& msg)
	{
		if (msg->conId.isRoom())
		{
			return;
		}

		auto conId = msg->conId;
		for(auto&& item : this | depthFirst)
		{
			if(item->data(Role::ConIdRole).value<Jmcpp::ConversationId>() == conId)
			{
				_updateMessagRole(item.current, msg);
				_addConversationToDB(msg);
				return;
			}
		}

		auto item = new QStandardItem();
		item->setData(QVariant::fromValue(conId), ConIdRole);
		item->setData(_co->getConversationImage(conId), Role::ImageRole);
		item->setData(getConversationDisplayName(conId), Role::TitleRole);
		item->setData(QDateTime::fromMSecsSinceEpoch(msg->time), Role::TimeRole);
		item->setData(getMessageDisplayString(msg), Role::MessageRole);
		this->appendRow(item);
		_updateItem(item);
		_updateMessagRole(item, msg);

		ConversationT con;
		con.conId = conId;
		qx::dao::save(con);
	}

	QStandardItem* ConversationModel::addConversationItem(Jmcpp::ConversationId const& conId)
	{
		for(auto&& item : this | depthFirst)
		{
			if(item->data(Role::ConIdRole).value<Jmcpp::ConversationId>() == conId)
			{
				return item.current;
			}
		}

		ConversationT con;
		con.conId = conId;

		auto item = new QStandardItem();
		item->setData(QVariant::fromValue(conId), ConIdRole);
		item->setData(_co->getConversationImage(conId), Role::ImageRole);
		item->setData(getConversationDisplayName(conId), Role::TitleRole);

		item->setData(QDateTime::currentDateTime(), Role::TimeRole);

		con.lastChatTime = QDateTime::currentDateTime();

		this->insertRow(0, item);
		_updateItem(item);
		_updateMessagRole(item);

		qx::dao::save(con);
		return item;
	}

	void ConversationModel::_addConversationToDB(Jmcpp::MessagePtr const& msg)
	{
		auto conId = msg->conId;
		ConversationT con;
		con.conId = conId;
		qx::dao::save(con);
	}


	None ConversationModel::_updateItem(QStandardItem* item)
	{
		auto index = QPersistentModelIndex(item->index());
		auto self = this | qTrack;
		auto co = _co;

		auto conId = item->data(Role::ConIdRole).value<Jmcpp::ConversationId>();

		auto pixmap = co_await co->getCacheAvatar(conId);

		co_await self;

		if(!index.isValid()){ co_return; }

		item = itemFromIndex(index);
		item->setData(pixmap, Role::ImageRole);

		auto name = co_await co->getDisplayName(conId, true);
		co_await self;

		if(!index.isValid()){ co_return; }

		item = itemFromIndex(index);
		item->setData(name, Role::TitleRole);

	}

	void ConversationModel::setSticktop(Jmcpp::ConversationId const& conId, bool enabled)
	{
		ConversationT con;
		con.conId = conId;
		auto err = qx::dao::fetch_by_id(con);
		con.sticktopTime = enabled ? QDateTime::currentDateTime() : QDateTime();
		qx::dao::save(con);
	}


	void ConversationModel::setMessageRole(Jmcpp::ConversationId const& conId, QString const& msg)
	{
		auto item = addConversationItem(conId);
		item->setData(msg, Role::MessageRole);
		//item->setData(QDateTime::currentDateTime(), Role::TimeRole);
	}

	void ConversationModel::updateMessageRole(Jmcpp::ConversationId const& conId)
	{
		auto item = addConversationItem(conId);
		_updateMessagRole(item);
	}

	void ConversationModel::setUnreadMessageCountRole(Jmcpp::ConversationId const& conId, int unreadMsgCount)
	{
		auto item = addConversationItem(conId);
		auto tmp = item->data(UnreadRole).toInt();
		item->setData(unreadMsgCount, ConversationModel::UnreadRole);

		if(tmp != unreadMsgCount)
		{
			ConversationT con;
			con.conId = conId;
			auto err = qx::dao::fetch_by_id(con);
			con.unreadMsgCount = unreadMsgCount;
			qx::dao::save(con);
		}
	}

	void ConversationModel::removeRow(int row)
	{
		auto item = this->item(row);
		auto conId = item->data(Role::ConIdRole).value<Jmcpp::ConversationId>();
		ConversationT con;
		con.conId = conId;
		qx::dao::delete_by_id(con);
		QStandardItemModel::removeRow(row);
	}

	void ConversationModel::remove(Jmcpp::ConversationId const& conId)
	{
		for(auto&& item : this | depthFirst)
		{
			if(item->data(Role::ConIdRole).value<Jmcpp::ConversationId>() == conId)
			{
				this->removeRow(item->row());
				break;
			}
		}
	}

	Q_SLOT None ConversationModel::onUserLogined()
	{
		std::vector<ConversationT> conList;
		qx::dao::fetch_all(conList);
		for(auto&& con : conList)
		{
			auto conId = con.conId;
			for(auto&& item : this | depthFirst)
			{
				if(item->data(Role::ConIdRole).value<Jmcpp::ConversationId>() == conId)
				{
					goto SKIP;
				}
			}

			auto item = new QStandardItem();
			item->setData(QVariant::fromValue(conId), ConIdRole);
			item->setData(_co->getConversationImage(conId), Role::ImageRole);

			item->setData(getConversationDisplayName(conId), Role::TitleRole);
			item->setData(con.lastChatTime, Role::TimeRole);

			//item->setData(con.lastMessageStr, Role::MessageRole);

			if(conId.isGroup())
			{
				item->setData(_co->isShield(conId.getGroupId()), Role::ShieldRole);
			}

			item->setData(_co->isNotDisturb(conId), NotDisturb);
			// 
			item->setData(con.unreadMsgCount, UnreadRole);
			item->setData(con.sticktopTime, Role::SticktopTimeRole);

			this->appendRow(item);

		SKIP:;

		}

		std::vector<QPersistentModelIndex> items;
		for(auto&& item : this | depthFirst)
		{
			items.emplace_back(item.current->index());
		}

		auto self = this | qTrack;
		auto co = _co;

		for(auto&& index : items)
		{
			if(!index.isValid()){ continue; }

			auto item = itemFromIndex(index);
			auto conId = item->data(Role::ConIdRole).value<Jmcpp::ConversationId>();

			_updateItem(item);
			_updateMessagRole(item);
		}
		co_return;
	}


	None ConversationModel::_updateMessagRole(QStandardItem* item)
	{
		auto self = this | qTrack;
		auto co = _co;
		auto conId = item->data(Role::ConIdRole).value<Jmcpp::ConversationId>();
		auto msg = co->getLatestMessage(conId);
		if(msg)
		{
			_updateMessagRole(item, msg);
		}
		else
		{

		}
		co_return;
	}


	None ConversationModel::_updateMessagRole(QStandardItem* item, Jmcpp::MessagePtr msg)
	{
		auto self = this | qTrack;
		auto co = _co;

		item->setData(QDateTime::fromMSecsSinceEpoch(msg->time), Role::TimeRole);

		if(msg->isOutgoing || !msg->conId.isGroup())
		{
			QString msgRole = getMessageDisplayString(msg);
			item->setData(msgRole, Role::MessageRole);
		}
		else
		{
			auto index = QPersistentModelIndex(item->index());
			auto senderInfo = co_await co->getCacheUserInfo(msg->sender);

			co_await self;
			if(index.isValid())
			{
				item = itemFromIndex(index);
				QString msgRole = getMessageDisplayString(msg, senderInfo);
				if(item->data(TimeRole).toDateTime().toMSecsSinceEpoch() == msg->time)
				{
					item->setData(msgRole, Role::MessageRole);
				}
			}
		}
		co_return;
	}

	void ConversationModel::onUserInfoUpdated(Jmcpp::UserId const& userId)
	{
		for(auto&& iter : this | depthFirst)
		{
			if(iter->data(Role::ConIdRole).value<Jmcpp::ConversationId>() == userId)
			{
				_updateItemUserInfo(iter.current, userId);
				break;
			}
		}
	}


	void ConversationModel::onGroupInfoUpdated(Jmcpp::GroupId groupId)
	{
		for(auto&& iter : this | depthFirst)
		{
			if(iter->data(Role::ConIdRole).value<Jmcpp::ConversationId>() == groupId)
			{
				_updateItemGroupInfo(iter.current, groupId);
				break;
			}
		}
	}

	None ConversationModel::_updateItemUserInfo(QStandardItem* item, Jmcpp::UserId userId)
	{
		auto self = this | qTrack;
		auto co = _co;

		auto index = QPersistentModelIndex(item->index());

		auto info = co_await co->getCacheUserInfo(userId);
		auto img = co_await co->getCacheUserAvatar(userId, info.avatar);
		co_await self;
		if(!index.isValid())
		{
			co_return;
		}

		item = self->itemFromIndex(index);
		item->setData(QVariant::fromValue(img), Role::ImageRole);
		item->setData(getUserDisplayName(info), Role::TitleRole);
	}

	None ConversationModel::_updateItemGroupInfo(QStandardItem* item, Jmcpp::GroupId groupId)
	{
		auto self = this | qTrack;
		auto co = _co;
		auto index = QPersistentModelIndex(item->index());

		auto info = co_await co->getCacheGroupInfo(groupId);
		auto img = co_await co->getCacheGroupAvatar(groupId, info.avatar);
		co_await self;
		if(!index.isValid())
		{
			co_return;
		}
		item = self->itemFromIndex(index);
		item->setData(QVariant::fromValue(img), Role::ImageRole);

		if(!info.groupName.empty())
		{
			item->setData(QString::fromStdString(info.groupName), Role::TitleRole);
		}
		else if(item->data(TitleRole).toString().isEmpty())
		{
			item->setData(QString::number(info.groupId.get()), Role::TitleRole);
		}
	}

	void ConversationModel::sortFn(const QModelIndex &source_left, const QModelIndex &source_right, bool& ret)
	{
		auto sticktopTime_left = source_left.data(ConversationModel::SticktopTimeRole).value<QDateTime>();
		auto sticktopTime_right = source_right.data(ConversationModel::SticktopTimeRole).value<QDateTime>();
		auto lastChatTime_left = source_left.data(ConversationModel::TimeRole).value<QDateTime>();
		auto lastChatTime_right = source_right.data(ConversationModel::TimeRole).value<QDateTime>();
		if(!sticktopTime_left.isNull() && !sticktopTime_right.isNull())
		{
			ret = sticktopTime_left < sticktopTime_right;
			return;
		}
		if(!sticktopTime_left.isNull() && sticktopTime_right.isNull())
		{
			ret = true;
			return;
		}

		if(sticktopTime_left.isNull() && sticktopTime_right.isNull())
		{
			ret = lastChatTime_left > lastChatTime_right;
			return;
		}

		if(sticktopTime_left.isNull() && !sticktopTime_right.isNull())
		{
			ret = false;
		}
	}


}
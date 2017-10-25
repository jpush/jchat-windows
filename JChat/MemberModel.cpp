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
			item->setText(u8"所有人");
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



	//////////////////////////////////////////////////////////////////////////

	void MemberDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		painter->save();

		QStyleOptionViewItem opt = option;
		initStyleOption(&opt, index);

		auto model = (MemberModel*) dynamic_cast<const QAbstractProxyModel*>(index.model())->sourceModel();

		auto font = painter->font();
		font.setPointSize(10);
		painter->setFont(font);
		painter->setPen(Qt::black);

		auto isOwner = index.data(MemberModel::Role::IsOwnerRole).toBool();

		auto name = index.data(MemberModel::Role::NameInfo).toString();

		{
			auto rect = opt.rect.adjusted(10, 10, 0, 0);
			rect.setSize({ 30,30 });

			QPixmap img = index.data(MemberModel::ImageRole).value<QPixmap>();
			painter->drawPixmap(rect, img);
		}

		{
			auto rect = opt.rect.adjusted(60, 0, 0, 0);

			QFontMetrics fm(painter->font());

			painter->drawText(rect, Qt::AlignVCenter, fm.elidedText(name, Qt::TextElideMode::ElideRight, opt.rect.width() - 120));
		}

		if(isOwner)
		{
			auto rect = opt.rect.adjusted(opt.rect.width() - 50, 13, 0, 0);
			rect.setSize({ 32,24 });
			static QPixmap img(u8":/image/resource/群主.png");
			painter->drawPixmap(rect, img);
		}
		else
		{
			if(model->isOwner())
			{
				if(opt.state&QStyle::StateFlag::State_MouseOver)
				{
					auto rect = opt.rect.adjusted(opt.rect.width() - 40, 17, 0, 0);
					rect.setSize({ 16,16 });

					static QPixmap img(u8":/image/resource/x常态.png");
					painter->drawPixmap(rect, img);
				}
			}
		}

		painter->restore();

		QStyledItemDelegate::paint(painter, option, index);
	}

	QSize MemberDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		auto size = QStyledItemDelegate::sizeHint(option, index);
		size.setHeight(50);
		return size;
	}

	bool MemberDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
	{
		if(event->type() == QEvent::MouseButtonRelease)
		{
			auto e = static_cast<QMouseEvent*>(event);
			auto m = (MemberModel*) dynamic_cast<const QAbstractProxyModel*>(index.model())->sourceModel();
			if(e->button() == Qt::LeftButton)
			{
				auto rect = option.rect.adjusted(10, 10, 0, 0);
				rect.setSize({ 30,30 });
				if(rect.contains(e->pos()))
				{
					Q_EMIT m->memberInfoClicked(index.data(MemberModel::UserIdRole).value<Jmcpp::UserId>());
				}

				if(m->isOwner())
				{
					auto isOwner = index.data(MemberModel::Role::IsOwnerRole).toBool();
					auto rect = option.rect.adjusted(option.rect.width() - 50, 17, 0, 0);
					rect.setSize({ 16,16 });
					if(rect.contains(e->pos()) && !isOwner)
					{
						Q_EMIT m->removeMemberClicked(index.data(MemberModel::UserIdRole).value<Jmcpp::UserId>());
					}
				}
			}
		}
		return false;
	}

} // namespace JChat

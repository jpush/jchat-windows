#include "RoomListModel.h"

#include <QScopedValueRollback>

#include "scope.hpp"

namespace JChat {

	static QPixmap defautAvatar()
	{
		return QPixmap{":/image/resource/Group.png"};
	}

	RoomListModel::RoomListModel(ClientObjectPtr const& co, QObject *parent)
		:QStandardItemModel(parent)
		, _co(co)
	{

	}

	RoomListModel::~RoomListModel()
	{

	}


	bool RoomListModel::canFetchMore(const QModelIndex &parent) const
	{
		return _canFetchMore && !_fetching && _co->isLogined() && _co->isConnected();
	}

	void RoomListModel::fetchMore(const QModelIndex &parent)
	{
		_fetchMore();
	}

	None RoomListModel::_fetchMore()
	{
		if(_fetching)
		{
			co_return;
		}

		auto self = this | qTrack;

		_fetching = true;
		SCOPE_EXIT(){ _fetching = false; };

		try
		{

			auto result = co_await _co->getAppRooms(_start);

			co_await self;
			_start += result.rooms.size();

			if(_start >= result.total)
			{
				_canFetchMore = false;
			}

			for(auto&& room : result.rooms)
			{
				auto item = new QStandardItem();
				item->setData(defautAvatar(), ImageRole);
				item->setData(QVariant::fromValue(room.roomId), RoomIdRole);
				item->setData(QString::fromStdString(room.roomName), NameRole);
				item->setData(QString::fromStdString(room.description), DescRole);
				this->appendRow(item);
			}

			co_return;
		}
		catch(std::runtime_error& e)
		{

		}

		co_await self;
	}

} // namespace JChat

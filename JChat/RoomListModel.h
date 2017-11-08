#pragma once


#include <QAbstractItemModel>
#include <QStandardItemModel>

#include "ClientObject.h"
namespace JChat {

	class RoomListModel : public QStandardItemModel
	{
		Q_OBJECT

	public:
		enum Role
		{
			RoomIdRole = Qt::UserRole + 1,
			NameRole,
			ImageRole,
			DescRole
		};

		RoomListModel(ClientObjectPtr const& co, QObject *parent);

		~RoomListModel();

		bool canFetchMore(const QModelIndex &parent) const override
		{
			return _canFetchMore && !_fetching && _co->isLogined() && _co->isConnected();
		}

		void fetchMore(const QModelIndex &parent) override
		{
			_fetchMore();
		}

		//////////////////////////////////////////////////////////////////////////

	protected:

		None _fetchMore();

	private:

		ClientObjectPtr _co;

		bool _fetching = false;
		bool _canFetchMore = true;

		size_t _start = 0;

	};

} // namespace JChat

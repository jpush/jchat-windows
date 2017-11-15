#include "GroupEventWidget.h"

#include "UserInfoWidget.h"
namespace JChat {

	GroupEventWidget::GroupEventWidget(QWidget *parent)
		: QListWidget(parent)
	{
		ui.setupUi(this);
	}

	GroupEventWidget::~GroupEventWidget()
	{
	}

	void GroupEventWidget::setClientObject(ClientObjectPtr const& co)
	{
		_co = co;

		connect(_co.get(), &ClientObject::userLogined, this, [=]
		{
			loadFromDB();
		});

		connect(_co.get(), &ClientObject::requestJoinGroupEvent, this, [=](Jmcpp::RequestJoinGroupEvent const& e)
		{

			if(e.bySelf)
			{
				GroupEventT et;
				et.isReject = false;

				et.eventId = e.eventId;
				et.bySelf = e.bySelf;
				et.groupId = e.groupId;
				et.message = e.requestMsg.c_str();

				et.fromUsername = e.fromUser.username;
				et.fromAppkey = e.fromUser.appKey;

				et.username = e.fromUser.username;
				et.appkey = e.fromUser.appKey;


				et.hasRead = false;
				et.status = GroupEventT::undone;
				et.time = QDateTime::fromMSecsSinceEpoch(getEventTime(e));


				qx::dao::save(et);

				updateItem(et);

			}
			else
			{
				for(auto&& u : e.users)
				{
					GroupEventT et;
					et.isReject = false;

					et.eventId = e.eventId;
					et.bySelf = e.bySelf;
					et.groupId = e.groupId;
					et.message = e.requestMsg.c_str();

					et.fromUsername = e.fromUser.username;
					et.fromAppkey = e.fromUser.appKey;

					et.username = u.username;
					et.appkey = u.appKey;

					et.hasRead = false;
					et.status = GroupEventT::undone;
					et.time = QDateTime::fromMSecsSinceEpoch(getEventTime(e));

					qx::dao::save(et);

					updateItem(et);
				}

			}

			Q_EMIT unreadChanged(unreadCount());

		});



		connect(_co.get(), &ClientObject::rejectJoinGroupEvent, this, [=](Jmcpp::RejectJoinGroupEvent const& e)
		{
			if(e.bySelf)
			{
				GroupEventT et;
				et.isReject = true;

				et.eventId = e.eventId;
				et.bySelf = e.bySelf;
				et.groupId = e.groupId;
				et.message = e.rejectReason.c_str();

				et.fromUsername = e.fromUser.username;
				et.fromAppkey = e.fromUser.appKey;

				et.username = e.fromUser.username;
				et.appkey = e.fromUser.appKey;

				et.hasRead = false;
				et.status = GroupEventT::undone;
				et.time = QDateTime::fromMSecsSinceEpoch(getEventTime(e));

				qx::dao::save(et);

				updateItem(et);

			}
			else
			{
				for(auto&& u : e.users)
				{
					GroupEventT et;
					et.isReject = true;

					et.eventId = e.eventId;
					et.bySelf = e.bySelf;
					et.groupId = e.groupId;
					et.message = e.rejectReason.c_str();

					et.fromUsername = e.fromUser.username;
					et.fromAppkey = e.fromUser.appKey;

					et.username = u.username;
					et.appkey = u.appKey;

					et.hasRead = false;
					et.status = GroupEventT::undone;
					et.time = QDateTime::fromMSecsSinceEpoch(getEventTime(e));

					qx::dao::save(et);

					updateItem(et);
				}

			}

			Q_EMIT unreadChanged(unreadCount());

		});
	}

	int GroupEventWidget::unreadCount() const
	{
		std::vector<GroupEventT> events;
		qx_query query;
		query.orderAsc("time");
		qx::dao::fetch_by_query(query, events);
		return std::count_if(events.begin(), events.end(), [](GroupEventT const& t)
		{
			return !t.hasRead;
		});
	}

	void GroupEventWidget::clearUnread()
	{
		qx_query query{ "update GroupEventT set hasRead = 1 " };
		qx::dao::call_query(query);

		Q_EMIT unreadChanged(0);
	}


	void GroupEventWidget::updateItem(GroupEventT const& et)
	{
		for(int i = 0, n = count(); i < n; i++)
		{
			auto item = this->item(i);
			if(item->data(Id).toLongLong() == et.id)
			{
				auto w = createItemWidget();
				w->setEvent(_co, et);

				setItemWidget(item, w);
				return;
			}
		}

		auto item = new QListWidgetItem();
		item->setSizeHint({ 0,64 });
		item->setData(Role::Id, et.id);

		auto w = createItemWidget();
		w->setEvent(_co, et);
		this->insertItem(0, item);
		setItemWidget(item, w);
	}

	void GroupEventWidget::loadFromDB()
	{
		std::vector<GroupEventT> events;
		qx_query query;
		query.orderAsc("time");

		qx::dao::fetch_by_query(query, events);

		for(auto&& e : events)
		{
			auto item = new QListWidgetItem();
			item->setSizeHint({ 0,64 });
			item->setData(Role::Id, e.eventId);

			auto w = createItemWidget();
			w->setEvent(_co, e);
			this->insertItem(0, item);
			setItemWidget(item, w);
		}

		Q_EMIT unreadChanged(std::count_if(events.begin(), events.end(), [](GroupEventT const& t)
		{
			return !t.hasRead;
		}));
	}

	JChat::GroupEventItem* GroupEventWidget::createItemWidget()
	{
		auto w = new GroupEventItem(this);

		connect(w, &GroupEventItem::infoClicked, this, [=](Jmcpp::UserId const& userId)
		{
			UserInfoWidget::showUserInfo(_co, userId, this->topLevelWidget());
		});

		connect(w, &GroupEventItem::passClicked, this, &GroupEventWidget::onPassClicked);

		connect(w, &GroupEventItem::rejectClicked, this, &GroupEventWidget::onRejectClicked);

		return w;
	}

	Q_SLOT None GroupEventWidget::onPassClicked(Jmcpp::GroupId groupId, int64_t eventId, Jmcpp::UserId const& user, Jmcpp::UserId const& fromUser)
	{
		auto w = static_cast<GroupEventItem*>(sender()) | qTrack;

		GroupEventT et = w->getGroupEventT();

		co_await _co->passJoinGroup(groupId, eventId, user, fromUser);

		if (et.id)
		{
			et.status = GroupEventT::passed;
			qx::dao::save(et);
		}

		co_await w;

		w->setStatus(et);
		w->setStatusLabel(u8"已同意");

		co_return;
	}

	Q_SLOT None GroupEventWidget::onRejectClicked(Jmcpp::GroupId groupId, int64_t eventId, Jmcpp::UserId const& user, Jmcpp::UserId const& fromUser)
	{
		auto w = static_cast<GroupEventItem*>(sender()) | qTrack;

		GroupEventT et = w->getGroupEventT();

		co_await _co->rejectJoinGroup(groupId, eventId, user, fromUser,"");

		if(et.id)
		{
			et.status = GroupEventT::rejected;
			qx::dao::save(et);
		}

		co_await w;

		w->setStatus(et);
		w->setStatusLabel(u8"已拒绝");

		co_return;
	}

} // namespace JChat

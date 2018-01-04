
#include "FriendEventListWidget.h"
#include <QPainter>

#include "FriendEventItemWidget.h"
#include "UserInfoWidget.h"
namespace JChat {

	FriendEventListWidget::FriendEventListWidget(QWidget *parent)
		: QListWidget(parent)

	{
		ui.setupUi(this);

	}

	FriendEventListWidget::~FriendEventListWidget()
	{
	}

	void FriendEventListWidget::setClientObject(ClientObjectPtr const& co)
	{
		Q_ASSERT(!_co);
		_co = co;

		connect(_co.get(), &ClientObject::userLogined, this, [=]
		{
			loadFromDB();
		});


		connect(_co.get(), &ClientObject::requestAddFriendEvent,
				this, [=](Jmcpp::RequestAddFriendEvent const& e)
		{

			auto userId = e.fromUser;
			std::vector<FriendEventT> events;

			qx_query query;

			query.where("username").isEqualTo(qx::cvt::to_variant(userId.username))
				.and_("appkey").isEqualTo(qx::cvt::to_variant(userId.appKey))
				.and_("isOutgoing").isEqualTo(false)
				.orderDesc("time")
				;

			qx::dao::fetch_by_query(query, events);

			FriendEventT et;

			if(!events.empty())
			{
				et = events.front();
			}
			et.username = userId.username;
			et.appkey = userId.appKey;
			et.message = e.message.c_str();
			et.isOutgoing = false;
			et.hasRead = false;
			et.status = FriendEventT::undone;
			et.time = QDateTime::fromMSecsSinceEpoch(getEventTime(e));
			qx::dao::save(et);
			updateItem(et);

			Q_EMIT unreadChanged(unreadCount());

		});


		connect(_co.get(), &ClientObject::passAddFriendEvent,
				this, [=](Jmcpp::PassAddFriendEvent const& e)
		{

			auto userId = e.fromUser;
			std::vector<FriendEventT> events;
			qx_query query;
			query.where("username").isEqualTo(qx::cvt::to_variant(userId.username))
				.and_("appkey").isEqualTo(qx::cvt::to_variant(userId.appKey))
				.and_("isOutgoing").isEqualTo(true)
				.orderDesc("time")
				;

			qx::dao::fetch_by_query(query, events);

			FriendEventT et;
			if(!events.empty())
			{
				et = events.front();
			}
			et.username = userId.username;
			et.appkey = userId.appKey;
			et.isOutgoing = true;
			et.hasRead = false;

			et.status = FriendEventT::passed;
			et.time = QDateTime::fromMSecsSinceEpoch(getEventTime(e));
			qx::dao::save(et);
			updateItem(et);

			Q_EMIT unreadChanged(unreadCount());

		});


		connect(_co.get(), &ClientObject::rejectAddFriendEvent,
				this, [=](Jmcpp::RejectAddFriendEvent const& e)
		{

			auto userId = e.fromUser;
			std::vector<FriendEventT> events;
			qx_query query;
			query.where("username").isEqualTo(qx::cvt::to_variant(userId.username))
				.and_("appkey").isEqualTo(qx::cvt::to_variant(userId.appKey))
				.and_("isOutgoing").isEqualTo(true)
				.orderDesc("time")
				;

			qx::dao::fetch_by_query(query, events);

			FriendEventT et;

			if(!events.empty())
			{
				et = events.front();
			}
			et.username = userId.username;
			et.appkey = userId.appKey;
			et.isOutgoing = true;
			et.hasRead = false;

			et.status = FriendEventT::rejected;
			et.time = QDateTime::fromMSecsSinceEpoch(getEventTime(e));
			qx::dao::save(et);
			updateItem(et);

			Q_EMIT unreadChanged(unreadCount());
		});


		connect(_co.get(), &ClientObject::requestAddFriendSent, this, [=](Jmcpp::UserId const& userId, bool isFriend)
		{
			std::vector<FriendEventT> events;
			qx_query query;
			query.where("username").isEqualTo(qx::cvt::to_variant(userId.username))
				.and_("appkey").isEqualTo(qx::cvt::to_variant(userId.appKey))
				.and_("isOutgoing").isEqualTo(true)
				.orderDesc("time")
				;

			qx::dao::fetch_by_query(query, events);

			FriendEventT et;
			if(!events.empty())
			{
				et = events.front();
			}
			et.username = userId.username;
			et.appkey = userId.appKey;
			et.isOutgoing = true;
			et.hasRead = true;

			et.status = isFriend ? FriendEventT::passed : FriendEventT::undone;
			et.time = QDateTime::currentDateTime();
			qx::dao::save(et);
			updateItem(et);
		});

	}

	int FriendEventListWidget::unreadCount() const
	{
		std::vector<FriendEventT> events;
		qx_query query;
		query.orderAsc("time");
		qx::dao::fetch_by_query(query, events);

		return std::count_if(events.begin(), events.end(), [](FriendEventT const& t)
		{
			return !t.hasRead;
		});
	}

	void FriendEventListWidget::clearUnread()
	{
		qx_query query{ "update FriendEventT set hasRead = 1 " };
		qx::dao::call_query(query);

		Q_EMIT unreadChanged(0);
	}

	void FriendEventListWidget::updateItem(FriendEventT const& et)
	{
		for(int i = 0, n = count(); i < n; i++)
		{
			auto item = this->item(i);
			if(item->data(EventId).toLongLong() == et.id)
			{
				auto w = createItemWidget();
				w->setEvent(_co, et);

				item = this->takeItem(i);
				insertItem(0, item);
				setItemWidget(item, w);
				return;
			}
		}

		auto item = new QListWidgetItem();
		item->setSizeHint({ 0,64 });
		item->setData(Role::EventId, et.id);

		auto w = createItemWidget();
		w->setEvent(_co, et);
		this->insertItem(0, item);
		setItemWidget(item, w);
	}

	void FriendEventListWidget::loadFromDB()
	{
		std::vector<FriendEventT> events;
		qx_query query;
		query.orderAsc("time");

		qx::dao::fetch_by_query(query, events);

		for(auto&& e : events)
		{
			auto item = new QListWidgetItem();
			item->setSizeHint({ 0,64 });
			item->setData(Role::EventId, e.id);

			auto w = createItemWidget();
			w->setEvent(_co, e);
			this->insertItem(0, item);
			setItemWidget(item, w);
		}

		Q_EMIT unreadChanged(std::count_if(events.begin(), events.end(), [](FriendEventT const& t)
		{
			return !t.hasRead;
		}));
	}

	FriendEventItemWidget* FriendEventListWidget::createItemWidget()
	{
		auto w = new FriendEventItemWidget(this);

		connect(w, &FriendEventItemWidget::infoClicked, this, [=](Jmcpp::UserId const& userId)
		{
			UserInfoWidget::showUserInfo(_co, userId, this->topLevelWidget());
		});

		connect(w, &FriendEventItemWidget::passClicked, this, &FriendEventListWidget::onPassClicked);

		connect(w, &FriendEventItemWidget::rejectClicked, this, &FriendEventListWidget::onRejectClicked);

		return w;
	}

	None FriendEventListWidget::onPassClicked(Jmcpp::UserId const& userId, int64_t id)
	{
		auto w = static_cast<FriendEventItemWidget*>(sender()) | qTrack;
		co_await _co->passAddFriend(userId);

		FriendEventT et;
		et.id = id;

		if(!qx::dao::fetch_by_id(et).isValid())
		{
			et.status = FriendEventT::passed;
			qx::dao::save(et);
		}

		co_await w;

		w->setStatus(false, FriendEventT::passed);
		w->setStatusLabel(u8"已同意");

	}

	None FriendEventListWidget::onRejectClicked(Jmcpp::UserId const& userId, int64_t id)
	{
		auto w = static_cast<FriendEventItemWidget*>(sender()) | qTrack;
		_co->rejectAddFriend(userId, "");

		FriendEventT et;
		et.id = id;

		if(!qx::dao::fetch_by_id(et).isValid())
		{
			et.status = FriendEventT::rejected;
			qx::dao::save(et);
		}

		co_await w;
		w->setStatus(false, FriendEventT::rejected);
		w->setStatusLabel(u8"已拒绝");
	}


	//////////////////////////////////////////////////////////////////////////
	void UnreadCountDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QStyleOptionViewItem opt = option;
		initStyleOption(&opt, index);

		auto unread = index.data(Qt::UserRole + 1).toInt();
		if(unread)
		{

			auto h = (opt.rect.height() - 16) / 2;
			auto rect = opt.rect.adjusted(opt.rect.width() - 50, h, 0, 0);
			rect.setSize({ 16,16 });
			QPixmap img;
			img.load(u8":/image/resource/MsgCountBkg.png");
			painter->drawPixmap(rect, img);

			painter->setPen(Qt::white);

			auto font = painter->font();
			font.setPointSize(font.pointSize() - 2);
			painter->setFont(font);

			painter->drawText(rect, Qt::AlignCenter, QString::number(unread));
		}

		QStyledItemDelegate::paint(painter, option, index);
	}

} // namespace JChat

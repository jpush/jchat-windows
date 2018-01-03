#include "UnreadUsers.h"
#include <QEventLoop>
#include <QStandardItemModel>
#include "ItemWidgetMapper.h"
#include "ChatIdItemWidget.h"
#include "BusyIndicator.h"

#include "Util.h"
namespace JChat {

	UnreadUsers::UnreadUsers(QWidget *parent)
		: QWidget(parent, Qt::Window)
	{
		ui.setupUi(this);
		setWindowModality(Qt::ApplicationModal);
		setAttribute(Qt::WA_DeleteOnClose);

		{
			_modelUnread = new QStandardItemModel(this);
			ui.listView->setModel(_modelUnread);

			auto mapper = new ItemWidgetMapper(this);
			mapper->setItemWidgetClass<ChatIdItemWidget>();
			mapper->addMapping(ChatIdItemWidget::Avatar, "avatar");
			mapper->addMapping(ChatIdItemWidget::Title, "title");
			mapper->addMapping(ChatIdItemWidget::CanCheck, "canCheck");
			mapper->addMapping(ChatIdItemWidget::CanClose, "canClose");
			mapper->setView(ui.listView);
		}

		{
			_modelRead = new QStandardItemModel(this);
			ui.listViewRead->setModel(_modelRead);

			auto mapper = new ItemWidgetMapper(this);
			mapper->setItemWidgetClass<ChatIdItemWidget>();
			mapper->addMapping(ChatIdItemWidget::Avatar, "avatar");
			mapper->addMapping(ChatIdItemWidget::Title, "title");
			mapper->addMapping(ChatIdItemWidget::CanCheck, "canCheck");
			mapper->addMapping(ChatIdItemWidget::CanClose, "canClose");
			mapper->setView(ui.listViewRead);
		}
	}

	UnreadUsers::~UnreadUsers()
	{
	}

	void UnreadUsers::closeEvent(QCloseEvent *event)
	{
		emit closed();
	}

	pplx::task<void> UnreadUsers::updateUsers(ClientObjectPtr co, int64_t msgId)
	{
		auto self = this | qTrack;
		try
		{
			auto result = co_await co->getMessageReceipts(msgId);

			co_await self;

			ui.labelUnread->setText(QString(u8"未读成员(%1)").arg(result.unreadUserList.size()));
			ui.labelRead->setText(QString(u8"已读成员(%1)").arg(result.readUserList.size()));


			for(auto&& userId : result.unreadUserList)
			{
				auto item = new QStandardItem();
				item->setData(false, ChatIdItemWidget::CanCheck);
				item->setData(false, ChatIdItemWidget::CanClose);

				item->setData(QVariant::fromValue(userId), ChatIdItemWidget::ConId);

				auto title = co_await co->getUserDisplayName(userId);
				co_await self;
				item->setData(title, ChatIdItemWidget::Title);

				auto pixmap = co_await co->getCacheUserAvatar(userId);
				co_await self;
				item->setData(pixmap, ChatIdItemWidget::Avatar);

				_modelUnread->appendRow(item);
			}

			for(auto&& userId : result.readUserList)
			{
				auto item = new QStandardItem();
				item->setData(false, ChatIdItemWidget::CanCheck);
				item->setData(false, ChatIdItemWidget::CanClose);

				item->setData(QVariant::fromValue(userId), ChatIdItemWidget::ConId);

				auto title = co_await co->getUserDisplayName(userId);
				co_await self;
				item->setData(title, ChatIdItemWidget::Title);

				auto pixmap = co_await co->getCacheUserAvatar(userId);
				co_await self;
				item->setData(pixmap, ChatIdItemWidget::Avatar);

				_modelRead->appendRow(item);
			}
		}
		catch(std::runtime_error& e)
		{

		}
	}

	void UnreadUsers::showUnreadUsers(ClientObjectPtr const& co, int64_t msgId, QWidget* parent)
	{
		auto w = new UnreadUsers(parent);
		BusyIndicator busy(parent->topLevelWidget());
		qAwait(w->updateUsers(co, msgId));
		w->show();
	}
} // namespace JChat

#include "BlackList.h"

#include "ItemWidgetMapper.h"
#include "ChatIdItemWidget.h"
#include "Util.h"
#include "BusyIndicator.h"


JChat::BlackList::BlackList(ClientObjectPtr const& co, QWidget *parent)
	: QWidget(parent, Qt::Window)
	, _co(co)
{
	ui.setupUi(this);

	setWindowModality(Qt::WindowModal);
	setAttribute(Qt::WA_DeleteOnClose);

	auto model = new QStandardItemModel(this);
	_model = model;

	ui.listView->setModel(_model);

	auto mapper = new ItemWidgetMapper(this);
	mapper->setItemWidgetClass<ChatIdItemWidget>();
	mapper->addMapping(ChatIdItemWidget::Avatar, "avatar");
	mapper->addMapping(ChatIdItemWidget::Title, "title");
	mapper->addMapping(ChatIdItemWidget::CanCheck, "canCheck");
	mapper->addMapping(ChatIdItemWidget::CanClose, "canClose");
	mapper->setView(ui.listView);

	connect(mapper, &ItemWidgetMapper::itemWidgetCreated, this, [=](QWidget* widget, QModelIndex const& index)
	{
		auto w = static_cast<ChatIdItemWidget*>(widget);
		auto idx = QPersistentModelIndex(index);
		connect(w, &ChatIdItemWidget::closeClicked, this, [=]
		{
			auto item = model->itemFromIndex(idx);
			onCloseClicked(item);
		});
	});
}

JChat::BlackList::~BlackList()
{
}

pplx::task<void>
JChat::BlackList::getBlackList()
{
	auto self = this | qTrack;
	auto co = _co;
	auto infos = co_await co->getBlackList();

	co_await self;
	_model->clear();

	for(auto&& info : infos)
	{
		auto item = new QStandardItem();
		item->setData(QVariant::fromValue(info.userId), ChatIdItemWidget::ConId);

		auto title = getUserDisplayName(info);
		item->setData(title, ChatIdItemWidget::Title);

		auto pixmap = co_await co->getCacheUserAvatar(info.userId, info.avatar);

		co_await self;

		item->setData(pixmap, ChatIdItemWidget::Avatar);
		item->setData(false, ChatIdItemWidget::CanCheck);
		item->setData(true, ChatIdItemWidget::CanClose);
		_model->appendRow(item);
	}
}

void
JChat::BlackList::onCloseClicked(QStandardItem* item)
{
	auto userId = item->data(ChatIdItemWidget::ConId).value<Jmcpp::UserId>();
	BusyIndicator busy(this);
	try
	{
		qAwait(_co->removeFromBlackList({ userId }));
		_model->removeRow(item->row());
	}
	catch(std::runtime_error& e)
	{

	}
}


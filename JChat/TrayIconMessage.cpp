#include "TrayIconMessage.h"

#include <QSystemTrayIcon>

#include "ItemWidgetMapper.h"
#include "TrayMessageItemWidget.h"

#include "ConversationModel.h"
#include "MainWidget.h"

JChat::TrayIconMessage::TrayIconMessage(QSystemTrayIcon* tray, QWidget *parent)
	: QWidget(parent)
	, _tray(tray)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowType::WindowStaysOnTopHint);


	auto mainWidget = MainWidget::getCurrentMainWidget();

	auto conModel = mainWidget->getConversationModel();

	auto model = new ProxyModel(this);
	model->setSourceModel(conModel);
	ui.listView->setModel(model);
	model->sort(0);
	model->setDataFnForRole(Qt::SizeHintRole, [](QModelIndex const& index, QVariant& data)
	{
		auto sz = data.toSize();
		sz.setHeight(50);
		data = sz;
	});
	model->setCompareFn(ConversationModel::sortFn);

	model->setFilterFn([=](int source_row, const QModelIndex &source_parent, bool& ret)
	{
		auto index = conModel->index(source_row, 0, source_parent);
		auto item = conModel->itemFromIndex(index);
		auto unreadCount = item->data(ConversationModel::UnreadRole).toInt();
		auto notDisturb = item->data(ConversationModel::NotDisturb).toBool();
		ret = unreadCount > 0 && !notDisturb;
	});


	connect(conModel, &ConversationModel::dataChanged, this, [=]
	{
		auto height = qMin(10, model->rowCount()) * 50 + 30;
		this->setFixedSize(width(), height);
	});



	auto mapper = new ItemWidgetMapper(this);
	mapper->setView(ui.listView);
	mapper->setItemWidgetClass<TrayMessageItemWidget>();

	mapper->addMapping(ConversationModel::ImageRole, "avatar");
	mapper->addMapping(ConversationModel::MessageRole, "message");
	mapper->addMapping(ConversationModel::TitleRole, "title");
	mapper->addMapping(ConversationModel::UnreadRole, "unreadMessageCount");


	connect(mapper, &ItemWidgetMapper::itemWidgetCreated, this, [=](QWidget* widget, QModelIndex const& index)
	{
		auto w = static_cast<TrayMessageItemWidget*>(widget);
		auto idx = QPersistentModelIndex(model->mapToSource(index));

	});


	connect(ui.listView, &QListView::clicked, this, [=](QModelIndex const& index)
	{
		auto conId = index.data(ConversationModel::Role::ConIdRole).value<Jmcpp::ConversationId>();

		Q_EMIT showMessage(conId);

	});


	connect(ui.toolButton, &QToolButton::clicked, this, [=]
	{
		Q_EMIT cancelFlash();
	});

}

JChat::TrayIconMessage::~TrayIconMessage()
{
}

void JChat::TrayIconMessage::show()
{
	auto geo = _tray->geometry();

	auto pt = geo.topLeft();

	pt -= {0, height()};

	move(pt);

	QWidget::show();
}

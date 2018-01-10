#include "SearchUser.h"

#include <QGraphicsDropShadowEffect>

#include "ConversationModel.h"
#include "ContactModel.h"
#include "ModelRange.h"
#include "Util.h"
#include "ItemWidgetMapper.h"
#include "ChatIdItemWidget.h"
#include "MainWidget.h"
#include "ProxyModel.h"
#include "Util.h"


JChat::SearchUser::SearchUser(QWidget *parent, bool multiChecked)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::WindowType::Popup);

	for(auto&& c : this->findChildren<QFrame*>())
	{
		c->setAttribute(Qt::WA_MacShowFocusRect, false);
	}
	for(auto&& c : this->findChildren<QLineEdit*>())
	{
		c->setAttribute(Qt::WA_MacShowFocusRect, false);
	}

	auto effect = new QGraphicsDropShadowEffect(this);
	effect->setOffset(2, 2);
	effect->setBlurRadius(10);
	this->setGraphicsEffect(effect);

	auto model = new QStandardItemModel(this);
	ui.listView->setModel(model);
	_model = model;

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


		connect(w, &ChatIdItemWidget::toggled, this, [=](bool checked)
		{
			auto item = model->itemFromIndex(idx);

			Q_EMIT itemChecked(item, checked);
			if(checked)
			{
				if(!multiChecked)
				{
					for(auto&& iter : model | depthFirst)
					{
						if(iter.current != item)
						{
							auto w = (ChatIdItemWidget*)ui.listView->indexWidget(iter->index());
							w->setChecked(false);
						}
					}
				}
			}
			else
			{
				//auto conId = item->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>();
			}
		});
	});


}

JChat::SearchUser::~SearchUser()
{
}

pplx::task<void>
JChat::SearchUser::searchUser(ClientObjectPtr co, QString const& text)
{
	_model->clear();

	auto self = this | qTrack;
	auto currentUserId = co->getCurrentUser();

	Jmcpp::ConversationId searched;
	try
	{
		auto info = co_await co->getCacheUserInfo({ text.toStdString(),currentUserId.appKey });
		searched = info.userId;
		auto image = co_await co->getCacheUserAvatar(info.userId, info.avatar);
		auto title = getUserDisplayName(info);

		co_await self;
		auto itemClone = new QStandardItem();
		itemClone->setData(QVariant::fromValue(Jmcpp::ConversationId(info.userId)), ChatIdItemWidget::ConId);
		itemClone->setData(image, ChatIdItemWidget::Avatar);
		itemClone->setData(title, ChatIdItemWidget::Title);
		itemClone->setData(true, ChatIdItemWidget::CanCheck);
		itemClone->setData(false, ChatIdItemWidget::CanClose);

		_model->appendRow(itemClone);
	}
	catch(Jmcpp::ServerException& e)
	{

	}

	auto mainWidget = MainWidget::getCurrentMainWidget();

	auto contactModel = qApp->property(ContactModel::staticMetaObject.className()).value<ContactModel*>();
	auto root = contactModel->getFriendRootItem();

	std::vector<Jmcpp::UserId> userIds;
	for(auto&& iter : root | depthFirst)
	{
		auto conId = iter->data(ContactModel::Role::ConIdRole).value<Jmcpp::ConversationId>();
		if(conId != searched)
		{
			userIds.emplace_back(conId.getUserId());
		}
	}

	auto name = text.toLower();
	for(auto&& userId : userIds)
	{
		try
		{
			auto info = co_await co->getCacheUserInfo(userId);
			auto image = co_await co->getCacheUserAvatar(info.userId, info.avatar);
			auto title = getUserDisplayName(info);

			auto username = QString::fromStdString(info.userId.username).toLower();
			auto nickname = QString::fromStdString(info.nickname).toLower();
			auto remark = QString::fromStdString(info.remark).toLower();

			if(remark.contains(name) || nickname.contains(name) || username.contains(name))
			{
				co_await self;

				auto itemClone = new QStandardItem();
				itemClone->setData(QVariant::fromValue(Jmcpp::ConversationId(info.userId)), ChatIdItemWidget::ConId);
				itemClone->setData(image, ChatIdItemWidget::Avatar);
				itemClone->setData(title, ChatIdItemWidget::Title);
				itemClone->setData(true, ChatIdItemWidget::CanCheck);
				itemClone->setData(false, ChatIdItemWidget::CanClose);
				_model->appendRow(itemClone);
			}
		}
		catch(std::runtime_error& e)
		{
		}
	}
}


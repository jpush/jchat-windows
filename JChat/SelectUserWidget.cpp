#include "SelectUserWidget.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

#include "ConversationModel.h"
#include "ContactModel.h"
#include "ModelRange.h"
#include "Util.h"
#include "ItemWidgetMapper.h"
#include "ChatIdItemWidget.h"
#include "MainWidget.h"
#include "ProxyModel.h"

#include "SearchUser.h"
#include "BusyIndicator.h"

JChat::SelectUserWidget::SelectUserWidget(ClientObjectPtr const&co, bool onlyFriend, bool multiSelect, QWidget *parent)
	: QWidget(parent, Qt::Window)
	, _co(co)
{
	ui.setupUi(this);
	setWindowModality(Qt::ApplicationModal);
	auto search = ui.lineEditSearch->addAction(QIcon(u8":/image/resource/ËÑË÷.png"), QLineEdit::TrailingPosition);

	auto model = new QStandardItemModel(this);
	ui.listView->setModel(model);

	auto modelSelect = new QStandardItemModel(this);

	ui.listViewSelected->setModel(modelSelect);

	{
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
				if(checked)
				{
					if(!multiSelect)
					{
						for(auto&& iter2 : model | depthFirst)
						{
							if(item != iter2.current)
							{
								auto w = (ChatIdItemWidget*)ui.listView->indexWidget(iter2->index());
								w->setChecked(false);
							}
						}
						modelSelect->clear();
					}

					auto itemClone = item->clone();
					itemClone->setData(false, ChatIdItemWidget::CanCheck);
					itemClone->setData(true, ChatIdItemWidget::CanClose);
					modelSelect->appendRow(itemClone);
				}
				else
				{
					auto conId = item->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>();
					for(auto&& iter : modelSelect | depthFirst)
					{
						if(iter->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>() == conId)
						{
							modelSelect->removeRow(iter->row());
							break;
						}
					}
				}

			});
		});
	}


	if(onlyFriend)
	{
		auto contactModel = qApp->property(ContactModel::staticMetaObject.className()).value<ContactModel*>();
		auto root = contactModel->getFriendRootItem();

		for(auto&& iter : root | depthFirst)
		{
			auto item = iter.current;

			auto itemClone = item->clone();

			itemClone->setData(itemClone->data(ContactModel::ConIdRole), ChatIdItemWidget::ConId);
			itemClone->setData(itemClone->data(ContactModel::ImageRole), ChatIdItemWidget::Avatar);
			itemClone->setData(itemClone->data(ContactModel::TitleRole), ChatIdItemWidget::Title);
			itemClone->setData(true, ChatIdItemWidget::CanCheck);
			itemClone->setData(false, ChatIdItemWidget::CanClose);

			model->appendRow(itemClone);
		}
	}
	else
	{
		auto mainWidget = MainWidget::getCurrentMainWidget();
		auto conPrxModel = mainWidget->getConversationProxyModel();
		auto conModel = (ConversationModel*)conPrxModel->sourceModel();;

		for(auto&& iter : conPrxModel | depthFirst)
		{
			auto index = conPrxModel->mapToSource(iter.current);
			auto item = conModel->itemFromIndex(index);

			auto itemClone = item->clone();

			itemClone->setData(itemClone->data(ConversationModel::ConIdRole), ChatIdItemWidget::ConId);
			itemClone->setData(itemClone->data(ConversationModel::ImageRole), ChatIdItemWidget::Avatar);
			itemClone->setData(itemClone->data(ConversationModel::TitleRole), ChatIdItemWidget::Title);
			itemClone->setData(true, ChatIdItemWidget::CanCheck);
			itemClone->setData(false, ChatIdItemWidget::CanClose);

			model->appendRow(itemClone);
		}
	}


	{
		auto mapper = new ItemWidgetMapper(this);
		mapper->setItemWidgetClass<ChatIdItemWidget>();
		mapper->addMapping(ChatIdItemWidget::Avatar, "avatar");
		mapper->addMapping(ChatIdItemWidget::Title, "title");
		mapper->addMapping(ChatIdItemWidget::CanCheck, "canCheck");
		mapper->addMapping(ChatIdItemWidget::CanClose, "canClose");
		mapper->setView(ui.listViewSelected);

		connect(mapper, &ItemWidgetMapper::itemWidgetCreated, this, [=](QWidget* widget, QModelIndex const& index)
		{
			auto w = static_cast<ChatIdItemWidget*>(widget);
			auto idx = QPersistentModelIndex(index);

			connect(w, &ChatIdItemWidget::closeClicked, this, [=]
			{
				auto item = modelSelect->itemFromIndex(idx);
				auto conId = item->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>();
				bool found = false;
				for(auto&& iter : model | depthFirst)
				{
					if(iter->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>() == conId)
					{
						auto w = (ChatIdItemWidget*)ui.listView->indexWidget(iter->index());
						w->setChecked(false);
						found = true;
						break;
					}
				}
				if(!found)
				{
					modelSelect->removeRow(item->row());
				}
			});
		});
	}


	auto searchUser = new SearchUser(this, multiSelect);
	connect(searchUser, &SearchUser::itemChecked, this, [=](QStandardItem* item, bool checked)
	{
		auto conId = item->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>();

		qDebug() << checked;
		if(checked)
		{
			bool found = false;
			for(auto&& iter : model | depthFirst)
			{
				if(iter->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>() == conId)
				{
					auto w = (ChatIdItemWidget*)ui.listView->indexWidget(iter->index());
					w->setChecked(true);
					found = true;
					break;
				}
			}
			if(!found)
			{
				if(!multiSelect)
				{
					for(auto&& iter2 : model | depthFirst)
					{
						if(iter2->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>() != conId)
						{
							auto w = (ChatIdItemWidget*)ui.listView->indexWidget(iter2->index());
							w->setChecked(false);
						}
					}
					modelSelect->clear();
				}

				auto itemClone = item->clone();
				itemClone->setData(false, ChatIdItemWidget::CanCheck);
				itemClone->setData(true, ChatIdItemWidget::CanClose);
				modelSelect->appendRow(itemClone);
			}
		}
		else
		{
			bool found = false;
			for(auto&& iter : model | depthFirst)
			{
				if(iter->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>() == conId)
				{
					auto w = (ChatIdItemWidget*)ui.listView->indexWidget(iter->index());
					w->setChecked(false);
					found = true;
					break;
				}
			}
			if(!found)
			{
				for(auto&& iter : modelSelect | depthFirst)
				{
					if(iter->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>() == conId)
					{
						modelSelect->removeRow(iter->row());
						break;
					}
				}
			}
		}
	});

	connect(search, &QAction::triggered, this, [=]
	{
		if(ui.lineEditSearch->text().isEmpty())
		{
			return;
		}
		auto self = this | qTrack;
		{
			//BusyIndicator busy(this);
			qAwait(searchUser->searchUser(_co, ui.lineEditSearch->text()));
		}
		if(!self || !this->isVisible())
		{
			return;
		}
		auto pos = ui.listView->mapToGlobal({ 0,0 });
		searchUser->setFixedSize(ui.listView->size());
		searchUser->move(pos);
		searchUser->show();
	});

}

JChat::SelectUserWidget::~SelectUserWidget()
{
}

std::vector<Jmcpp::ConversationId>
JChat::SelectUserWidget::_getConversationIds() const
{
	std::vector<Jmcpp::ConversationId> results;
	auto modelSelected = static_cast<QStandardItemModel*>(ui.listViewSelected->model());
	for(auto&& iter : modelSelected | depthFirst)
	{
		results.emplace_back(iter->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>());
	}

	return results;
}


void
JChat::SelectUserWidget::closeEvent(QCloseEvent *event)
{
	emit closed();
}

std::vector<Jmcpp::ConversationId>
JChat::SelectUserWidget::getConversationIds(ClientObjectPtr const&co, QWidget* parent /*= nullptr*/, bool onlyFriend /*= false*/, bool multiSelect /*= true*/)
{
	QEventLoop el;
	SelectUserWidget w(co, onlyFriend, multiSelect, parent);
	connect(w.ui.btnOK, &QPushButton::clicked, [&]
	{
		el.exit(0);
	});

	connect(w.ui.btnCancel, &QPushButton::clicked, [&]
	{
		el.exit(1);
	});
	connect(&w, &SelectUserWidget::closed, [&]
	{
		el.exit(1);
	});


	w.show();
	if(el.exec() == 0)
	{
		return w._getConversationIds();
	}

	return {};
}



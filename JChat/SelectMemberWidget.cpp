#include "SelectMemberWidget.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QMessageBox>


#include "ContactModel.h"
#include "ModelRange.h"
#include "Util.h"
#include "ItemWidgetMapper.h"
#include "ChatIdItemWidget.h"

#include "ImageCut.h"
#include "SearchUser.h"
#include "BusyIndicator.h"

//////////////////////////////////////////////////////////////////////////
JChat::SelectMemberWidget::SelectMemberWidget(ClientObjectPtr const&co, QWidget *parent)
	: QWidget(parent, Qt::Window)
	, _co(co)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Window | Qt::WindowType::CustomizeWindowHint | Qt::WindowType::WindowCloseButtonHint);
	setWindowModality(Qt::ApplicationModal);

	ui.labelAvatar->installEventFilter(this);


	ui.stackedWidget->setCurrentIndex(0);

	auto search = ui.lineEditSearch->addAction(QIcon(u8":/image/resource/搜索.png"), QLineEdit::TrailingPosition);

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

	auto contactModel = qApp->property(ContactModel::staticMetaObject.className()).value<ContactModel*>();
	auto root = contactModel->getFriendRootItem();

	for(auto&& iter : root | depthFirst)
	{
		auto itemClone = iter->clone();

		auto info = itemClone->data(ContactModel::InfoRole).value<Jmcpp::UserInfo>();

		itemClone->setData(itemClone->data(ContactModel::ConIdRole), ChatIdItemWidget::ConId);
		itemClone->setData(itemClone->data(ContactModel::ImageRole), ChatIdItemWidget::Avatar);
		itemClone->setData(itemClone->data(ContactModel::TitleRole), ChatIdItemWidget::Title);
		itemClone->setData(true, ChatIdItemWidget::CanCheck);
		itemClone->setData(false, ChatIdItemWidget::CanClose);

		model->appendRow(itemClone);
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

				for(auto&& iter : model | depthFirst)
				{
					if(iter->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>() == conId)
					{
						auto w = (ChatIdItemWidget*)ui.listView->indexWidget(iter->index());
						w->setChecked(false);
						break;
					}
				}
			});
		});
	}

	auto searchUser = new SearchUser(this);
	connect(searchUser, &SearchUser::itemChecked, this, [=](QStandardItem* item, bool checked)
	{
		auto conId = item->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>();
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
				auto itemClone = item->clone();
				itemClone->setData(false, ChatIdItemWidget::CanCheck);
				itemClone->setData(true, ChatIdItemWidget::CanClose);
				modelSelect->appendRow(itemClone);
			}
		}
		else
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

JChat::SelectMemberWidget::~SelectMemberWidget()
{
}

void JChat::SelectMemberWidget::closeEvent(QCloseEvent *event)
{
	emit closed();
}


std::optional< std::vector<Jmcpp::UserId>>
JChat::SelectMemberWidget::getUserIds(ClientObjectPtr const&co, QString const& title, QString& groupName, QWidget* parent /*= nullptr*/)
{
	QEventLoop el;
	SelectMemberWidget w(co, parent);
	w.setWindowTitle(title);
	w.setAdvanceGroupEnable(false);

	w.setGroupNameEnable(!groupName.isEmpty());


	connect(w.ui.btnOK, &QPushButton::clicked, [&]
	{
		el.exit(0);
	});

	connect(w.ui.btnCancel, &QPushButton::clicked, [&]
	{
		el.exit(1);
	});
	connect(&w, &SelectMemberWidget::closed, [&]
	{
		el.exit(1);
	});

	w.show();
	if(el.exec() == 0)
	{
		groupName = w.ui.lineEdit->text();

		return w._getUserIds();
	}

	return {};
}

std::optional<std::vector<Jmcpp::UserId>>
JChat::SelectMemberWidget::getUserIds(ClientObjectPtr const&co, QString const& title,
									  QString& groupName, QString &groupAvatar, bool& isPublic,
									  QWidget* parent /*= nullptr*/)
{
	QEventLoop el;
	SelectMemberWidget w(co, parent);
	w.setWindowTitle(title);
	w.setAdvanceGroupEnable(true);
	w.setGroupNameEnable(false);

	connect(w.ui.btnDone, &QPushButton::clicked, [&]
	{
		if(w.ui.lineEditGroupName->text().isEmpty())
		{
			QMessageBox::warning(&w, "", u8"请输入群组名称", QMessageBox::Ok);
			return;
		}

		el.exit(0);
	});

	connect(w.ui.btnCancel, &QPushButton::clicked, [&]
	{
		el.exit(1);
	});
	connect(&w, &SelectMemberWidget::closed, [&]
	{
		el.exit(1);
	});

	w.show();
	if(el.exec() == 0)
	{
		isPublic = w.ui.radioButtonPublic->isChecked();
		groupName = w.ui.lineEditGroupName->text();

		auto tmp = QDir::temp();
		auto tmpImge = tmp.absoluteFilePath(QString("JChat_%1.jpg").arg(QString::number(QDateTime::currentMSecsSinceEpoch())));
		if(w.ui.labelAvatar->pixmap()->save(tmpImge, "JPG"))
		{
			groupAvatar = tmpImge;
		}

		return w._getUserIds();
	}

	return {};
}

bool
JChat::SelectMemberWidget::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == ui.labelAvatar && event->type() == QEvent::MouseButtonRelease)
	{
		auto e = static_cast<QMouseEvent*>(event);
		if(e->button() == Qt::LeftButton)
		{
			auto img = ImageCut::cutImage(this, ui.labelAvatar->pixmap()->toImage());

			if(!img.isNull())
			{
				ui.labelAvatar->setPixmap(QPixmap::fromImage(img));
			}
		}
	}

	return false;
}

Q_SLOT void
JChat::SelectMemberWidget::on_btnNext_clicked()
{
	ui.stackedWidget->setCurrentIndex(1);
}

Q_SLOT void
JChat::SelectMemberWidget::on_btnPrev_clicked()
{
	ui.stackedWidget->setCurrentIndex(0);
}

Q_SLOT
void JChat::SelectMemberWidget::on_radioButton_toggled(bool checked)
{
	ui.label_6->setText(checked ? u8"私有群：只能通过群成员邀请入群，无需审核" : u8"公开群：用户可主动申请入群，需群主审核");
}

std::vector<Jmcpp::UserId>
JChat::SelectMemberWidget::_getUserIds()
{
	std::vector<Jmcpp::UserId> results;
	auto modelSelected = static_cast<QStandardItemModel*>(ui.listViewSelected->model());
	for(auto&& iter : modelSelected | depthFirst)
	{
		results.emplace_back(iter->data(ChatIdItemWidget::ConId).value<Jmcpp::ConversationId>().getUserId());
	}

	return results;
}

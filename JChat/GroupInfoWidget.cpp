#include "GroupInfoWidget.h"
#include <QPainter>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QMenu>

#include "UserInfoWidget.h"
#include "MemberModel.h"
#include "ProxyModel.h"

#include "BusyIndicator.h"
#include "GroupInfoWidget.h"

#include "SelectMemberWidget.h"
#include "ItemWidgetMapper.h"

#include "MemberItemWidget.h"

namespace JChat {

	GroupInfoWidget::GroupInfoWidget(ClientObjectPtr const& co, Jmcpp::GroupId groupId, QWidget *parent)
		: QWidget(parent)
		, _co(co)
		, _groupId(groupId)
	{
		ui.setupUi(this);

		_memberModel = new MemberModel(co, groupId, this);

		connect(_memberModel, &MemberModel::rowsInserted, this, [=]
		{
			ui.labelMemberCount->setText(QString(u8"成员:%1").arg(_memberModel->rowCount()));
		});
		connect(_memberModel, &MemberModel::rowsRemoved, this, [=]
		{
			ui.labelMemberCount->setText(QString(u8"成员:%1").arg(_memberModel->rowCount()));
		});

		connect(_co.get(), &ClientObject::notDisturbChanged, this, [=](Jmcpp::ConversationId const& conId, bool on)
		{
			if(conId == _groupId)
			{
				QSignalBlocker blocker(ui.checkBoxNoDis);
				ui.checkBoxNoDis->setChecked(on);
			}
		});

		connect(_co.get(), &ClientObject::groupShieldChanged, this, [=](Jmcpp::GroupId groupId, bool on)
		{
			if(groupId == _groupId)
			{
				QSignalBlocker blocker(ui.checkBoxShield);
				ui.checkBoxShield->setChecked(on);
			}
		});

		connect(_co.get(), &ClientObject::groupInfoUpdatedEvent, this, [=](Jmcpp::GroupInfoUpdatedEvent const& e)
		{
			if(e.groupId != _groupId)
			{
				return;
			}
			updateInfo();
		});

		auto model = new ProxyModel(this);
		model->setSourceModel(_memberModel);
		model->setFilterRole(MemberModel::NameInfo);

		ui.listView->setModel(model);

		auto mapper = new ItemWidgetMapper(this);

		connect(mapper, &ItemWidgetMapper::itemWidgetCreated, this, [=](QWidget* widget, QModelIndex const& index)
		{
			auto w = static_cast<MemberItemWidget*>(widget);

			auto idx = QPersistentModelIndex(index);

			auto userId = index.data(MemberModel::UserIdRole).value<Jmcpp::UserId>();

			connect(w, &QWidget::customContextMenuRequested, this, [=](QPoint const& pt)
			{
				if(!idx.isValid())
				{
					return;
				}

				auto isSlient = idx.data(MemberModel::IsSlientRole).toBool();

				if(_memberModel->isOwner())
				{
					QPoint globalPos = widget->mapToGlobal(pt);

					QMenu myMenu;

					myMenu.addAction(isSlient ? u8"解除禁言" : u8"禁言", this, [=]
					{
						_co->setGroupMemberSilent(groupId, userId, !isSlient);
					});

					myMenu.addAction(u8"移出群聊", this, [=]
					{
						removeMember({ userId });
					});

					myMenu.exec(globalPos);
				}

			});

			connect(w, &MemberItemWidget::memberInfoClicked, this, [=]()
			{
				UserInfoWidget::showUserInfo(_co, userId, this);
			});
		});


		mapper->addMapping(MemberModel::NameInfo, "title");
		mapper->addMapping(MemberModel::ImageRole, "avatar");
		mapper->addMapping(MemberModel::IsOwnerRole, "isOwner");
		mapper->addMapping(MemberModel::IsSlientRole, "isSilent");

		mapper->setItemWidgetClass<MemberItemWidget>();

		mapper->setView(ui.listView);


		ui.lineEditGroupName->installEventFilter(this);
		ui.textEditGroupDesc->installEventFilter(this);

		connect(qApp, &QApplication::focusChanged, this, [=](QWidget *old, QWidget *now)
		{
			if(!isVisible() || !now){ return; }
			while(now){
				if(now == this){ return; }
				now = now->parentWidget();
			}
			close();
		});

		auto effect = new QGraphicsDropShadowEffect(this);
		effect->setOffset(-2, 0);
		effect->setBlurRadius(10);
		this->setGraphicsEffect(effect);
		connect(ui.btnClose, &QToolButton::clicked, this, &GroupInfoWidget::close);
		parent->installEventFilter(this);

		connect(ui.lineEditSearch, &QLineEdit::textChanged, this, [=](QString const& text)
		{
			model->setFilterWildcard(text);
		});

		_memberModel->updateItems();

		updateInfo();
	}

	GroupInfoWidget::~GroupInfoWidget()
	{

	}

	Q_SLOT void GroupInfoWidget::close()
	{
		if(active)
		{
			return;
		}
		active = true;
		raise();
		auto parent = parentWidget();
		auto prarentWidth = parent->width();

		QPropertyAnimation *animnow = new QPropertyAnimation(this, "pos", this);
		animnow->setDuration(300);
		animnow->setEasingCurve(QEasingCurve::Type::InOutCubic);
		animnow->setStartValue(QPoint(prarentWidth - width(), 0));
		animnow->setEndValue(QPoint(prarentWidth + 30, 0));
		QObject::connect(animnow, &QPropertyAnimation::finished, this, [=]{
			animnow->deleteLater();
			lower();
			QWidget::close();
			active = false;
		});
		animnow->start();
	}

	void GroupInfoWidget::on_checkBoxNoDis_toggled(bool checked)
	{
		try
		{
			//BusyIndicator busy(this);
			qAwait(_co->setNotDisturb(_groupId, checked));
		}
		catch(std::runtime_error& e)
		{
			QSignalBlocker blocker(ui.checkBoxNoDis);
			ui.checkBoxNoDis->setChecked(!checked);
		}
	}

	void GroupInfoWidget::on_checkBoxShield_toggled(bool checked)
	{
		try
		{
			//BusyIndicator busy(this);
			qAwait(_co->setGroupShield(_groupId, checked));
		}
		catch(std::runtime_error& e)
		{
			QSignalBlocker blocker(ui.checkBoxShield);
			ui.checkBoxShield->setChecked(!checked);
		}
	}

	void GroupInfoWidget::on_btnQuitGroup_clicked()
	{
		if(QMessageBox::warning(this, "", u8"确定要退出群吗?",
								QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		{
			try
			{
				//BusyIndicator busy(this);
				qAwait(_co->exitGroup(_groupId));
			}
			catch(std::runtime_error& e)
			{
			}
		}
	}

	void GroupInfoWidget::on_btnAddMember_clicked()
	{
		auto userIds = SelectMemberWidget::getUserIds(_co, u8"添加群成员", nullptr, this);
		if(!userIds){ return; }

		BusyIndicator busy(this);
		bool failed = false;
		for(auto&& userId : *userIds)
		{
			try
			{
				qAwait(_co->addGroupMembers(_groupId, { userId }));
			}
			catch(Jmcpp::ServerException& e)
			{
				if(e.code() == 880607)
				{
					continue;
				}
				failed = true;
				QMessageBox::warning(this, "", e.what(), QMessageBox::Ok);
				break;
			}
			catch(std::runtime_error& e)
			{
				failed = true;
				QMessageBox::warning(this, "", e.what(), QMessageBox::Ok);
				break;
			}
		}

		if(!failed)
		{
			QMessageBox::warning(this, "", u8"添加群成员成功", QMessageBox::Ok);
		}

	}

	void GroupInfoWidget::removeMember(Jmcpp::UserIdList const& userIds)
	{
		auto self = this | qTrack;

		BusyIndicator busy(this->topLevelWidget());
		try
		{
			auto name = qAwait(_co->getUserDisplayName(userIds.front()));
			if(QMessageBox::question(this, u8"删除群成员", QString(u8"确定将 %1 移出群聊吗").arg(name), QMessageBox::Cancel | QMessageBox::Default, QMessageBox::Ok) == QMessageBox::Ok)
			{
				qAwait(_co->removeGroupMembers(_groupId, userIds));
			}
		}
		catch(std::runtime_error& e)
		{
		}

	}

	None GroupInfoWidget::updateInfo()
	{
		auto self = this | qTrack;

		auto info = co_await _co->getCacheGroupInfo(_groupId);

		co_await self;


		if(!info.groupName.empty())
			ui.lineEditGroupName->setText(info.groupName.data());
		ui.textEditGroupDesc->setText(info.description.data());

		QSignalBlocker b(ui.checkBoxNoDis), b2(ui.checkBoxShield);
		ui.checkBoxNoDis->setChecked(_co->isNotDisturb(_groupId));
		ui.checkBoxShield->setChecked(_co->isShield(_groupId));
	}

	bool GroupInfoWidget::eventFilter(QObject *watched, QEvent *event)
	{
		if(watched == parentWidget() && event->type() == QEvent::Resize)
		{
			auto ev = static_cast<QResizeEvent*>(event);
			move(ev->size().width() - width(), 0);
			resize(width(), ev->size().height());
		}

		if(watched == ui.lineEditGroupName || watched == ui.textEditGroupDesc)
		{
			switch(event->type())
			{
				case QEvent::FocusOut:
				{
					if(ui.lineEditGroupName->isModified() || ui.textEditGroupDesc->document()->isModified())
					{
						_co->updateGroupInfo(_groupId,
											 ui.lineEditGroupName->text().toStdString(),
											 ui.textEditGroupDesc->toPlainText().toStdString()).then([](pplx::task<void> t)
						{
							try
							{
								t.get();
							}
							catch(...)
							{
							}
						});
					}
				}break;
				default:
					break;
			}
		}

		return false;
	}

	void GroupInfoWidget::paintEvent(QPaintEvent *event)
	{
		QStyleOption opt;
		opt.init(this);
		QPainter p(this);
		style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	}


	void GroupInfoWidget::showEvent(QShowEvent *event)
	{
		if(active)
		{
			return;
		}

		active = true;
		auto parent = parentWidget();
		auto prarentWidth = parent->width();

		resize(width(), parent->height());

		setFocus();
		raise();

		QPropertyAnimation *animnow = new QPropertyAnimation(this, "pos", this);

		animnow->setDuration(300);
		animnow->setEasingCurve(QEasingCurve::Type::InOutCubic);
		animnow->setStartValue(QPoint(prarentWidth + 30, 0));
		animnow->setEndValue(QPoint(prarentWidth - width(), 0));
		QObject::connect(animnow, &QPropertyAnimation::finished, this, [=]{
			animnow->deleteLater();
			active = false;
		});
		animnow->start();
	}

} // namespace JChat

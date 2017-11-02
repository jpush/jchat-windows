
#include "UserInfoWidget.h"

#include <QMessageBox>
#include <QMouseEvent>
#include <QFileDialog>

#include "BusyIndicator.h"
#include "MainWidget.h"
#include "RequestMessageWidget.h"

#include "SelectUserWidget.h"

#include "ImageCut.h"

namespace JChat
{
	UserInfoWidget::UserInfoWidget(ClientObjectPtr const& co, QWidget *parent /*= Q_NULLPTR*/) : QWidget(parent)
		, _co(co)
	{
		ui.setupUi(this);

		this->setAttribute(Qt::WA_DeleteOnClose);
		this->setWindowFlags(Qt::Window | Qt::WindowType::CustomizeWindowHint | Qt::WindowType::WindowCloseButtonHint);
		this->setFixedSize(size());

		ui.label->installEventFilter(this);

		setEditable(false);

		auto menu = new QMenu(this);

		menu->addAction(u8"发送名片", [=]
		{
			auto conIds = SelectUserWidget::getConversationIds(_co, this);
			auto mainWidget = MainWidget::getCurrentMainWidget();
			if(mainWidget)
			{
				for(auto&& conId : conIds)
				{
					mainWidget->getOrCreateChatWidget(conId)->sendUserCard(_userId);
				}
				if(!conIds.empty())
				{
					//QMessageBox::information(this->topLevelWidget(), "", u8"成功", QMessageBox::Ok);
				}
			}

		});

		auto notDisturb = menu->addAction(u8"消息免打扰", [=](bool checked)
		{
			try
			{
				BusyIndicator busy(this);
				qAwait(co->setNotDisturb(_userId, checked));
			}
			catch(std::runtime_error& e)
			{

			}

		});
		notDisturb->setCheckable(true);

		auto blackList = menu->addAction(u8"加入黑名单", [=](bool checked)
		{
			try
			{
				BusyIndicator busy(this);
				if(checked)
				{
					qAwait(co->addToBlackList({ _userId }));
				}
				else
				{
					qAwait(co->removeFromBlackList({ _userId }));
				}
			}
			catch(std::runtime_error& e)
			{

			}
		});
		blackList->setCheckable(true);


		connect(menu, &QMenu::aboutToShow, [=]()
		{
			QSignalBlocker block(notDisturb);
			notDisturb->setChecked(co->isNotDisturb(_userId));

			QSignalBlocker block2(blackList);
			blackList->setChecked(co->isInBlackList(_userId));

		});


		ui.btnMenu->setMenu(menu);

		connect(ui.toolButton, &QToolButton::clicked, this, [=]
		{
			ui.lineEditRemark->setEnabled(true);
			ui.lineEditRemark->setFocus();
		});

		connect(ui.lineEditRemark, &QLineEdit::editingFinished, this, [=]
		{
			if(ui.lineEditRemark->text().isEmpty())
			{
				QMessageBox::warning(this, "", u8"备注不能为空", QMessageBox::Ok);
				ui.lineEditRemark->setFocus();
				return;
			}

			try
			{
				BusyIndicator busy(this);
				qAwait(_co->updateFriendRemark(_userId, ui.lineEditRemark->text().toStdString(), ""));
				ui.lineEditRemark->setEnabled(false);
			}
			catch(std::runtime_error& e)
			{
				ui.lineEditRemark->setEnabled(true);
			}
		});

		connect(ui.toolButtonEdit, &QToolButton::clicked, this, [=]
		{
			setEditable(true);

			ui.toolButtonEdit->hide();
			ui.toolButtonSave->show();
			ui.toolButtonCancel->show();
		});

		connect(ui.toolButtonSave, &QToolButton::clicked, this, [=]
		{
			try
			{
				BusyIndicator busy(this);
				qAwait(updateSelfInfo());
				ui.labelNickName->setText(ui.lineEditNickName->text());

				setEditable(false);

				ui.toolButtonEdit->show();
				ui.toolButtonSave->hide();
				ui.toolButtonCancel->hide();
			}
			catch(std::runtime_error& e)
			{

			}
		});

		connect(ui.toolButtonCancel, &QToolButton::clicked, this, [=]
		{
			setUserInfo(_userInfo);
			setEditable(false);

			ui.toolButtonEdit->show();
			ui.toolButtonSave->hide();
			ui.toolButtonCancel->hide();
		});

		connect(ui.toolButtonAddFriend, &QToolButton::clicked, this, [=]
		{
			auto text = RequestMessageWidget::getText(this);
			if(!text.isEmpty())
			{
				BusyIndicator busy(this);
				try
				{
					qAwait(_co->addFriend(_userId, text.toStdString()));

				}
				catch(std::runtime_error& e)
				{

				}
			}
		});

		connect(ui.toolButtonSendMessage, &QToolButton::clicked, this, [=]
		{
			auto mainW = qApp->property(MainWidget::staticMetaObject.className()).value<MainWidget*>();
			if(mainW)
			{
				close();
				mainW->switchToConversation(_userId);
			}
		});
	}

	UserInfoWidget::~UserInfoWidget()
	{
	}


	void UserInfoWidget::setEditable(bool editable)
	{
		_editable = editable;
		{
			auto children = this->findChildren<QLineEdit*>();
			for(auto&& child : children)
			{
				child->setEnabled(editable);
			}
		}

		{
			auto children = this->findChildren<QComboBox*>();
			for(auto&& child : children)
			{
				child->setEnabled(editable);
			}
		}
	}

	void UserInfoWidget::setMode(Mode m)
	{
		_mode = m;
		switch(m)
		{
			case self:
			{
				ui.btnMenu->hide();

				ui.frame->hide();

				ui.toolButtonAddFriend->hide();
				ui.toolButtonSendMessage->hide();

				ui.toolButtonCancel->hide();
				ui.toolButtonSave->hide();

			}break;
			case friends:
			{
				ui.toolButtonAddFriend->hide();
				ui.toolButtonEdit->hide();
				ui.toolButtonCancel->hide();
				ui.toolButtonSave->hide();

			}break;
			case stranger:
			{
				ui.frame->hide();

				ui.toolButtonEdit->hide();
				ui.toolButtonCancel->hide();
				ui.toolButtonSave->hide();
			}break;
			default:
				break;
		}
	}

	void UserInfoWidget::setUserInfo(Jmcpp::UserInfo const& userInfo)
	{
		_userInfo = userInfo;
		_userId = userInfo.userId;

		ui.lineEditSignature->setText(userInfo.signature.data());

		ui.labelNickName->setText(userInfo.nickname.data());

		ui.lineEditUserName->setText(userInfo.userId.username.data());
		ui.lineEditNickName->setText(userInfo.nickname.data());

		ui.lineEditRemark->setText(userInfo.remark.data());

		ui.comboBox->setCurrentIndex((int)userInfo.gender);

		ui.lineEditRegion->setText(userInfo.region.data());
	}

	pplx::task<void> UserInfoWidget::updateSelfInfo()
	{
		Jmcpp::UpdateUserInfoParam param;
		param.nickname = ui.lineEditNickName->text().toStdString();
		param.signature = ui.lineEditSignature->text().toStdString();

		param.gender = (Jmcpp::Gender)ui.comboBox->currentIndex();
		param.region = ui.lineEditRegion->text().toStdString();

		return _co->updateSelfInfo(param);
	}

	void UserInfoWidget::showUserInfo(ClientObjectPtr const& co, Jmcpp::UserId const& userId, QWidget* parent /*= nullptr*/)
	{
		try
		{
			BusyIndicator busy(parent);
			auto info = qAwait(co->getCacheUserInfo(userId));
			auto pixmap = qAwait(co->getCacheUserAvatar(userId));

			if(info.userId == co->getCurrentUser())
			{
				auto w = new UserInfoWidget(co, parent);

				w->setWindowModality(Qt::WindowModality::ApplicationModal);
				w->setMode(UserInfoWidget::self);
				w->show();

				w->setUserInfo(info);
				w->setAvatar(pixmap);
			}
			else
			{
				auto w = new UserInfoWidget(co, parent);
				w->setWindowModality(Qt::WindowModality::ApplicationModal);
				if(co->isFriend(info.userId))
				{
					w->setMode(UserInfoWidget::friends);
				}
				else
				{
					w->setMode(UserInfoWidget::stranger);
				}
				w->show();
				w->setUserInfo(info);
				w->setAvatar(pixmap);
			}

			busy.close();
		}
		catch(Jmcpp::ServerException& e)
		{
			if(e.code() == 882002)
			{
				QMessageBox::warning(parent, u8"提示", u8"用户不存在", QMessageBox::Ok);
			}
			else
			{
				QMessageBox::warning(parent, u8"提示", e.what(), QMessageBox::Ok);
			}
		}
		catch(std::runtime_error& e)
		{

		}
	}

	void UserInfoWidget::showSelfInfo(ClientObjectPtr const& co, QWidget* parent /*= nullptr*/)
	{
		auto w = new UserInfoWidget(co, parent) | qTrack;
		try
		{
			w->setWindowModality(Qt::WindowModality::ApplicationModal);
			w->setMode(UserInfoWidget::self);
			w->show();

			auto info = qAwait(co->getSelfInfo());
			auto pixmap = qAwait(co->getCacheUserAvatar(co->getCurrentUser()));
			if(w)
			{
				w->setUserInfo(info);
				w->setAvatar(pixmap);
			}

		}
		catch(std::runtime_error& e)
		{
			if(w)
			{
				w->close();
				w->deleteLater();
			}
		}
	}

	bool UserInfoWidget::eventFilter(QObject *watched, QEvent *event)
	{
		if(watched == ui.label && event->type() == QEvent::MouseButtonRelease)
		{
			if(_mode == self)
			{
				QMouseEvent * ev = (QMouseEvent*)event;
				if(ev->button() == Qt::LeftButton)
				{
					try
					{
						BusyIndicator busy(this);

						auto pixmap = qAwait(_co->getCacheUserAvatar(_userId));
						auto image = ImageCut::cutImage(this, pixmap.toImage());
						if(image.isNull())
						{
							return false;
						}


						auto tmp = QDir::temp();
						auto tmpImge = tmp.absoluteFilePath(QString("JChat_%1.jpg").arg(QString::number(QDateTime::currentMSecsSinceEpoch())));
						if(image.save(tmpImge, "JPG"))
						{
							auto mediaId = qAwait(_co->updateSelfAvatar(tmpImge.toStdString()));
							auto pixmap = qAwait(_co->getCacheUserAvatar(_userId, mediaId, true));
							ui.label->setPixmap(pixmap);

							QFile::remove(tmpImge);
						}
					}
					catch(std::runtime_error& e)
					{
						qWarning() << e.what();
					}
				}
			}
		}
		return false;

	}

}
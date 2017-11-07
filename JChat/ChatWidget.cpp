#include "ChatWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QPointer>
#include <QKeyEvent>
#include <QFontDatabase>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QTextTable>
#include <QUuid>
#include <QCompleter>
#include <QScrollBar>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>

#include "ItemWidgetTextLeft.h"
#include "ItemWidgetTextRight.h"

#include "ItemWidgetImageRight.h"
#include "ItemWidgetFileRight.h"
#include "ItemWidgetUserCardRight.h"

#include "UserInfoWidget.h"

#include "Emoji.h"
#include "EmojiPicker.h"
#include "SelectMemberWidget.h"
#include "BusyIndicator.h"
#include "MainWidget.h"
#include "FileManager.h"

#include "SelectUserWidget.h"

#include "Binding.h"

namespace JChat
{

	ChatWidget::ChatWidget(ClientObjectPtr const& co, Jmcpp::ConversationId const& conId, QWidget *parent /*= Q_NULLPTR*/)
		:QWidget(parent)
		, _co(co)
		, _conId(conId)
	{
		ui.setupUi(this);
		ui.listWidget->setClientObject(_co);
		ui.textEdit->installEventFilter(this);

		if(_conId.isUser())
		{
			ui.btnSetting->hide();
		}
		else
		{
			ui.btnName->setEnabled(false);
			ui.btnAdd->hide();
			_groupInfo = new GroupInfoWidget(_co, _conId.getGroupId(), this);
			auto memberModel = _groupInfo->memberModel();
			_groupInfo->hide();

			_completer = new QCompleter(this);
			_completer->setWrapAround(false);
			_completer->setModel(memberModel->getCompleterModel());

			ui.textEdit->setCompleter(_completer);
		}

		connect(ui.textEdit, &QTextEdit::textChanged, this, [=]
		{
			//ui.btnSend->setEnabled(!ui.textEdit->toPlainText().isEmpty());
		});


		auto fileManager = new FileManager(this);
		fileManager->hide();
		connect(ui.btnFileManager, &QToolButton::clicked, this, [fileManager]
		{
			fileManager->show();
		});

		connect(_co.get(), &ClientObject::selfInfoUpdated, this, &ChatWidget::onSelfInfoUpdated);
		connect(_co.get(), &ClientObject::userInfoUpdated, this, &ChatWidget::onUserInfoUpdated);
		connect(_co.get(), &ClientObject::groupInfoUpdated, this, &ChatWidget::onGroupInfoUpdated);

		init();

		//////////////////////////////////////////////////////////////////////////
		_emojiPicker = new EmojiPicker(this);
		connect(_emojiPicker, &EmojiPicker::emojiSelected, this, &ChatWidget::onEmojiSelected);
		connect(_emojiPicker, &EmojiPicker::largeEmojiSelected, this, &ChatWidget::onLargetEmojiSelected);


		connect(ui.btnEmoji, &QToolButton::clicked, this, [=]()
		{
			auto cursor = ui.textEdit->textCursor();
			auto point = ui.listWidget->mapToGlobal({ 0,ui.listWidget->height() });
			auto geo = _emojiPicker->frameGeometry();
			point -= { 0, geo.height()};
			_emojiPicker->move(point);
			_emojiPicker->show();
		});

		connect(ui.listWidget->verticalScrollBar(), &QScrollBar::valueChanged, [=](int value){
			if(value == 0){
				loadMoreMessage(10);
			}
		});
		ui.listWidget->verticalScrollBar()->installEventFilter(this);

	}

	ChatWidget::~ChatWidget()
	{
	}


	QString ChatWidget::getEditingText() const
	{
		std::optional<Jmcpp::UserIdList> unused;
		return ui.textEdit->getText(unused);
	}

	Q_SLOT void ChatWidget::on_btnName_clicked()
	{
		if(_conId.isUser())
		{
			auto w = new UserInfoWidget(_co, this);
			try
			{
				w->setWindowModality(Qt::WindowModality::ApplicationModal);
				if(_co->isFriend(_conId.getUserId()))
				{
					w->setMode(UserInfoWidget::friends);
				}
				else
				{
					w->setMode(UserInfoWidget::stranger);
				}
				w->show();
				auto info = qAwait(_co->getCacheUserInfo(_conId.getUserId()));
				auto pixmap = qAwait(_co->getCacheUserAvatar(_conId.getUserId()));
				w->setUserInfo(info);
				w->setAvatar(pixmap);
			}
			catch(std::runtime_error& e)
			{
				w->close();
				w->deleteLater();
			}
		}
	}

	Q_SLOT void ChatWidget::on_btnSetting_clicked()
	{
		_groupInfo->show();
	}

	Q_SLOT void ChatWidget::on_btnAdd_clicked()
	{
		QString groupName = "xxx";
		auto userIds = SelectMemberWidget::getUserIds(_co, u8"多人会话", groupName, this);
		if(!userIds)
		{
			return;
		}

		if(groupName.isEmpty())
		{
			QStringList names;
			for(auto&& userId : *userIds){
				names << userId.username.c_str();
			}
			groupName = names.join(',').left(30);
		}

		BusyIndicator busy(this);
		Jmcpp::ConversationId conId;
		try
		{
			auto info = qAwait(_co->createGroup(groupName.toStdString()));
			conId = info.groupId;
			qAwait(_co->addGroupMembers(info.groupId, *userIds));
		}
		catch(std::runtime_error& e)
		{
			QMessageBox::warning(this->topLevelWidget(), "", e.what(), QMessageBox::Ok);
		}

		if(conId.isGroup())
		{
			auto mainWidget = qApp->property(MainWidget::staticMetaObject.className()).value<MainWidget*>();
			if(mainWidget)
				mainWidget->switchToConversation(conId);
		}
	}

	Q_SLOT None ChatWidget::on_btnSend_clicked()
	{
		if(ui.textEdit->toPlainText().isEmpty())
		{
			co_return;
		}

		auto html = ui.textEdit->toHtml();

		std::optional<Jmcpp::UserIdList> userList;
		auto text = ui.textEdit->getText(userList);

		auto iw = new ItemWidgetTextRight() | qTrack;
		iw->setDateTime(QDateTime::currentDateTime());
		iw->setHtml(html);
		ui.listWidget->insertItemWidget(iw);
		iw->setProgress(0);

		ui.textEdit->clear();

		auto co = _co;
		auto conId = _conId;

		auto content = co_await co->createTextContent(text.toStdString());

		Jmcpp::MessagePtr msg;
		if(conId.isGroup())
		{
			msg = co->buildMessage(conId.getGroupId(), content, {}, userList);
		}
		else
		{
			msg = co->buildMessage(conId.getUserId(), content);
		}

		try
		{
			co_await co->sendMessage(msg);
			co_await iw;
			iw->setMessage(msg);
			iw->setComplete();
			iw->setUnreadUserCount(msg->unreadUserCount);
			return;
		}
		catch(std::exception& e)
		{
			qDebug() << e.what();
		}
		co_await iw;
		iw->setFailed();
	}

	Q_SLOT void ChatWidget::on_btnImage_clicked()
	{
		auto filePath = QFileDialog::getOpenFileName(this, tr("Open File"), "",
													 tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)"));
		if(filePath.isEmpty())
		{
			return;
		}
		sendImage(filePath);
	}


	Q_SLOT void ChatWidget::on_btnFile_clicked()
	{
		auto filePath = QFileDialog::getOpenFileName(this);
		if(filePath.isEmpty())
		{
			return;
		}

		sendFile(filePath);
	}


	Q_SLOT void ChatWidget::on_btnUserCard_clicked()
	{
		auto users = SelectUserWidget::getConversationIds(_co, this->topLevelWidget(), true, false);
		if(users.empty())
		{
			return;
		}

		sendUserCard(users.front().getUserId());
	}

	None ChatWidget::init()
	{
		auto self = (this) | qTrack;

		if(_conId.isUser())
		{
			ui.btnName->setText(_conId.getUserId().username.data());

			auto img = co_await _co->getCacheUserAvatar(_conId.getUserId());

			co_await self;
			ui.listWidget->setLeftAvatar(img);

			auto name = co_await _co->getUserDisplayName(_conId.getUserId());
			co_await self;

			ui.listWidget->setLeftDisplayName(name);
			ui.btnName->setText(name);

			// 			auto user = _co->createUser(_conId.getUserId(),this);
			// 			new Binding(user, "displayName", ui.btnName, "text");
			// 
			// 			connect(user, &User::avatarFilePathChanged, this, [=](QString const& filePath)
			// 			{
			// 				QPixmap img(filePath);
			// 				ui.listWidget->setLeftAvatar(img);
			// 			});
		}
		else
		{
			ui.btnName->setText(QString::number(_conId.getGroupId().get()));
			auto info = co_await _co->getCacheGroupInfo(_conId.getGroupId());
			co_await self;
			if(!info.groupName.empty())
			{
				ui.btnName->setText(info.groupName.data());
			}
		}

		auto img = co_await _co->getCacheUserAvatar(_co->getCurrentUser());
		co_await self;
		ui.listWidget->setRightAvatar(img);
	}

	None ChatWidget::forwardMessage(Jmcpp::MessagePtr const& msgPtr)
	{
		auto self = this | qTrack;
		auto iw = ui.listWidget->insertMessage(msgPtr);
		auto widget = iw->asWidget() | qTrack;
		try
		{
			auto msg = co_await _co->sendMessage(msgPtr);
			co_await widget;
			iw->setMessage(msg);
			iw->setComplete();
			iw->setUnreadUserCount(msg->unreadUserCount);
			co_return;
		}
		catch(...)
		{
		}
		co_await widget;
		iw->setFailed();
		co_return;
	}

	None ChatWidget::sendImage(QImage const& image)
	{
		auto co = _co;
		auto iw = new ItemWidgetImageRight() | qTrack;
		iw->setDateTime(QDateTime::currentDateTime());
		iw->setImage(QPixmap::fromImage(image));
		ui.listWidget->insertItemWidget(iw);
		iw->setProgress(0);

		try
		{
			auto img = image;

			co_await ResumeBackground{};

			QByteArray ba;
			QBuffer buffer(&ba);
			buffer.open(QIODevice::WriteOnly);
			img.save(&buffer, "JPG");

			auto imageName = QUuid::createUuid().toString() + ".jpg";
			auto filePath = _co->imageFilePath(imageName);
			{
				QFile file(co->imageFilePath(imageName));
				if(file.open(QFile::WriteOnly))
				{
					img.save(&file, "JPEG");
				}
			}

			co_await iw;
			iw->setFilePath(filePath);

			auto content = co_await co->createImageContent((const char*)ba.constData(), ba.size(),
														   "snapshot.jpg");

			if(auto newPath = co->imageFilePath(content); QFile::rename(filePath, newPath))
			{
				co_await iw;
				iw->setFilePath(newPath);
			}

			Jmcpp::MessagePtr msg;
			if(_conId.isGroup())
			{
				msg = co->buildMessage(_conId.getGroupId(), content);
			}
			else
			{
				msg = co->buildMessage(_conId.getUserId(), content);
			}

			co_await co->sendMessage(msg);
			co_await iw;

			iw->setMessage(msg);
			iw->setComplete();
			iw->setUnreadUserCount(msg->unreadUserCount);
			co_return;
		}
		catch(std::system_error& e)
		{

		}
		catch(Jmcpp::ServerException& e)
		{
		}

		co_await iw;
		iw->setFailed();
	}

	None ChatWidget::sendImage(QString const& filePath)
	{
		if(QFileInfo(filePath).size() > 8 * 1024 * 1024)
		{
			QMessageBox::warning(this, "", u8"文件过大!");
			co_return;
		}

		auto iw = new ItemWidgetImageRight() | qTrack;
		iw->setDateTime(QDateTime::currentDateTime());
		iw->setFilePath(filePath);
		QPixmap img(filePath);
		auto movie = new QMovie(filePath);
		if(movie->frameCount() > 1)
		{
			iw->setMovie(movie, img.size());
		}
		else
		{
			delete movie;
			iw->setImage(img);
		}

		ui.listWidget->insertItemWidget(iw);
		iw->setProgress(0);

		try
		{
			auto content = co_await _co->createImageContent(filePath.toStdString());
			Jmcpp::MessagePtr msg;
			if(_conId.isGroup())
			{
				msg = _co->buildMessage(_conId.getGroupId(), content);
			}
			else
			{
				msg = _co->buildMessage(_conId.getUserId(), content);
			}

			co_await _co->sendMessage(msg);
			co_await iw;

			iw->setMessage(msg);
			iw->setComplete();
			iw->setUnreadUserCount(msg->unreadUserCount);

			co_return;
		}
		catch(std::system_error& e)
		{

		}
		catch(Jmcpp::ServerException& e)
		{
		}

		co_await iw;
		iw->setFailed();
	}

	None ChatWidget::sendFile(QString const& filePath)
	{
		if(QFileInfo(filePath).size() > 8 * 1024 * 1024)
		{
			QMessageBox::warning(this, "", u8"文件过大!");
			co_return;
		}

		auto iw = new ItemWidgetFileRight() | qTrack;
		iw->setDateTime(QDateTime::currentDateTime());

		QFileInfo info(filePath);

		iw->setFilePath(filePath);
		iw->setFileName(info.fileName());
		iw->setFileSize(info.size());
		ui.listWidget->insertItemWidget(iw);
		iw->setProgress(0);

		try
		{
			auto content = co_await _co->createFileContent(filePath.toStdString());
			Jmcpp::MessagePtr msg;
			if(_conId.isGroup())
			{
				msg = _co->buildMessage(_conId.getGroupId(), content);
			}
			else
			{
				msg = _co->buildMessage(_conId.getUserId(), content);
			}

			co_await _co->sendMessage(msg);
			co_await iw;

			iw->setMessage(msg);
			iw->setComplete();
			iw->setUnreadUserCount(msg->unreadUserCount);

			co_return;
		}
		catch(std::system_error& e)
		{

		}
		catch(Jmcpp::ServerException& e)
		{

		}
		co_await iw;
		iw->setFailed();
	}

	None ChatWidget::sendUserCard(Jmcpp::UserId userId)
	{
		auto co = _co;
		auto iw = new ItemWidgetUserCardRight() | qTrack;
		iw->setCardUserId(userId);
		ui.listWidget->insertItemWidget(iw);
		iw->setProgress(0);

		auto content = co_await co->createTextContent(u8"推荐一张名片给你");
		QJsonObject extrasData;
		extrasData["userName"] = QString::fromStdString(userId.username);
		extrasData["appKey"] = QString::fromStdString(userId.appKey);
		extrasData["businessCard"] = "businessCard";

		auto extras = QJsonDocument(extrasData).toJson().toStdString();

		content.extras = extras;

		Jmcpp::MessagePtr msg;
		if(_conId.isGroup())
		{
			msg = co->buildMessage(_conId.getGroupId(), content);
		}
		else
		{
			msg = co->buildMessage(_conId.getUserId(), content);
		}

		try
		{
			co_await co->sendMessage(msg);

			auto imgTask = co->getCacheAvatar(userId);
			auto infoTask = co->getCacheUserInfo(userId);

			auto img = co_await imgTask;
			auto info = co_await infoTask;

			co_await iw;
			iw->setCardUserInfo(info);
			iw->setCardAvatar(img);

			iw->setMessage(msg);
			iw->setComplete();
			iw->setUnreadUserCount(msg->unreadUserCount);
			co_return;
		}
		catch(...)
		{

		}
		co_await iw;
		iw->setFailed();
		co_return;
	}

	None ChatWidget::sendLargeEmoji(QString const& filePath, QString const& emojiName)
	{
		if(QFileInfo(filePath).size() > 8 * 1024 * 1024)
		{
			QMessageBox::warning(this, "", u8"文件过大!");
			co_return;
		}

		auto iw = new ItemWidgetImageRight() | qTrack;
		iw->setDateTime(QDateTime::currentDateTime());

		QPixmap img(filePath);
		auto movie = new QMovie(filePath);
		if(movie->frameCount() > 1)
		{
			iw->setMovie(movie, img.size());
		}
		else
		{
			delete movie;
			img = img.scaledToWidth(120);
			iw->setImage(img);
		}

		ui.listWidget->insertItemWidget(iw);
		iw->setProgress(0);

		try
		{
			QFile file(filePath);
			if(!file.open(QFile::ReadOnly))
			{
				iw->setFailed();
				co_return;
			}

			auto data = file.readAll();
			auto content = co_await _co->createImageContent(data.constData(), data.size(), file.fileName().toStdString());

			content.extras = R"_({"kLargeEmoticon":"kLargeEmoticon"})_";

			Jmcpp::MessagePtr msg = _co->buildMessage(_conId, content);

			co_await _co->sendMessage(msg);
			co_await iw;

			iw->setMessage(msg);
			iw->setComplete();
			iw->setUnreadUserCount(msg->unreadUserCount);

			co_return;
		}
		catch(std::system_error& e)
		{

		}
		catch(Jmcpp::ServerException& e)
		{
		}

		co_await iw;
		iw->setFailed();
	}

	void ChatWidget::onEmojiSelected(QString const& emojiHtml)
	{
		ui.textEdit->insertHtml(emojiHtml);
		_emojiPicker->hide();
		ui.textEdit->setFocus();
	}

	void ChatWidget::onLargetEmojiSelected(QString const& filePath)
	{
		sendLargeEmoji(filePath, "");
		_emojiPicker->hide();
		ui.textEdit->setFocus();
	}

	None ChatWidget::loadMoreMessage(size_t count)
	{
		if(listWidget()->property("allLoaded").toBool())
		{
			co_return;
		}
		if(_loadingMessage)
		{
			co_return;
		}
		_loadingMessage = true;

		auto self = this | qTrack;

		auto msg = listWidget()->getFirstLoadedMessage();
		try
		{
			listWidget()->setLoadingMessageIndicator(true);

			auto msgs = co_await _co->getMessagesAsync(_conId, count, msg);
			co_await self;

			listWidget()->setLoadingMessageIndicator(false);

			for(auto&& m : msgs)
			{
				ui.listWidget->insertMessage(m, 0);
				qApp->processEvents();
			}

			if(msgs.size() < count)
			{
				//listWidget()->insertCenterWidget(u8"没有更多消息", 0);
				listWidget()->setProperty("allLoaded", true);
			}

			ui.listWidget->verticalScrollBar()->setValue(1);
		}
		catch(...)
		{

		}

		co_await self;
		_loadingMessage = false;
		listWidget()->setLoadingMessageIndicator(false);
	}

	None ChatWidget::loadMessage(size_t count)
	{
		if(_loadingMessage)
		{
			co_return;
		}
		_loadingMessage = true;

		auto self = this | qTrack;

		auto msg = listWidget()->getFirstLoadedMessage();
		try
		{
			listWidget()->setLoadingMessageIndicator(true);

			auto msgs = co_await _co->getMessagesAsync(_conId, count, msg);
			co_await self;

			listWidget()->setLoadingMessageIndicator(false);

			for(auto&& m : msgs)
			{
				ui.listWidget->insertMessage(m, 0);
			}

			if(msgs.size() < count)
			{
				if(!listWidget()->property("allLoaded").toBool())
				{
					//listWidget()->insertCenterWidget(u8"没有更多消息", 0);
					listWidget()->setProperty("allLoaded", true);
				}
			}

			ui.listWidget->scrollToBottom();

		}
		catch(...)
		{

		}

		co_await self;
		_loadingMessage = false;
		listWidget()->setLoadingMessageIndicator(false);
	}

	None ChatWidget::onSelfInfoUpdated(Jmcpp::UserId const& userId)
	{
		auto self = this | qTrack;
		auto img = co_await _co->getCacheUserAvatar(userId);
		co_await self;
		ui.listWidget->setRightAvatar(img);
	}

	None ChatWidget::onUserInfoUpdated(Jmcpp::UserId const& userId)
	{
		if(_conId == userId)
		{
			auto self = this | qTrack;

			auto img = co_await _co->getCacheUserAvatar(_conId.getUserId());
			co_await self;
			ui.listWidget->setLeftAvatar(img);

			auto name = co_await _co->getUserDisplayName(_conId.getUserId());
			co_await self;

			ui.listWidget->setLeftDisplayName(name);
			ui.btnName->setText(name);
		}
	}

	None ChatWidget::onGroupInfoUpdated(Jmcpp::GroupId groupId)
	{
		if(_conId == groupId)
		{
			auto self = this | qTrack;
			auto info = co_await _co->getCacheGroupInfo(_conId.getGroupId());
			co_await self;

			if(!info.groupName.empty())
				ui.btnName->setText(info.groupName.data());
		}
	}

	void ChatWidget::showEvent(QShowEvent *event)
	{
		auto msg = listWidget()->getFirstLoadedMessage();
		if(!msg)
		{
			loadMessage(10);
		}
	}

	void ChatWidget::hideEvent(QHideEvent *event)
	{

	}

	bool ChatWidget::eventFilter(QObject *watched, QEvent *event)
	{
		if(watched == ui.textEdit)
		{
			switch(event->type())
			{
				case QEvent::KeyPress:
				{
					QKeyEvent* ev = static_cast<QKeyEvent*>(event);

					switch(ev->key())
					{
						case Qt::Key_Enter:
						case Qt::Key_Return:
						{
							if(ev->modifiers() & Qt::Modifier::CTRL)
							{
								ui.textEdit->insertPlainText("\n");
							}
							else
							{
								ui.btnSend->click();
							}
							return true;
						}break;
						default:
							break;
					}
				}break;
				default:
					break;
			}
		}
		else if(watched == ui.listWidget->verticalScrollBar() && event->type() == QEvent::Wheel)
		{
			auto e = static_cast<QWheelEvent*>(event);
			if(ui.listWidget->verticalScrollBar()->value() == 0 && e->angleDelta().y() > 0)
			{
				loadMoreMessage(10);
			}
		}
		return false;
	}
}
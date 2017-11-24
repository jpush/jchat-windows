
#include "MessageListWidget.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>

#include "ItemWidgetCenter.h"

#include "ItemWidgetTextLeft.h"
#include "ItemWidgetTextRight.h"

#include "ItemWidgetImageRight.h"
#include "ItemWidgetImageLeft.h"

#include "ItemWidgetFileLeft.h"
#include "ItemWidgetFileRight.h"

#include "ItemWidgetLocationLeft.h"
#include "ItemWidgetLocationRight.h"

#include "ItemWidgetUserCardLeft.h"
#include "ItemWidgetUserCardRight.h"

#include "ItemWidgetVoiceLeft.h"
#include "ItemWidgetVoiceRight.h"

#include "Util.h"
#include "Emoji.h"

#include "SelectUserWidget.h"
#include "MainWidget.h"
#include "UnreadUsers.h"


namespace JChat
{
	MessageListWidget::MessageListWidget(QWidget *parent) : QListWidget(parent)
	{
		ui.setupUi(this);

		_loadIndicator = new QLabel(this);
		_loadIndicator->setFixedSize(30, 30);
		_loadIndicator->setScaledContents(true);
		_loadIndicator->setStyleSheet(" background-color: rgba(0, 0, 0,0 ); ");
		_loadIndicator->hide();

		_timer = new QTimer(this);
		_timer->setSingleShot(true);
		connect(_timer, &QTimer::timeout, this, [=]
		{
			updateVisibleItems();
		});

		connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [=]
		{
			if(!isVisible())
			{
				return;
			}
			_timer->start(300);
		});

		connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, [=]
		{
			if(!isVisible())
			{
				return;
			}
			if(!_timer->isActive())
			{
				_timer->start(300);
			}
		});

		connect(this, &MessageListWidget::visibleChanged, this, [=](QVector<ItemWidgetInterface*> const& visible,
																	QVector<ItemWidgetInterface*> const& invisible)
		{
			sendMessageReceipts(visible, ItemWidgetInterface::ItemFlags(ItemWidgetInterface::type_mask) & ~ItemWidgetInterface::voice);
		});
	}

	MessageListWidget::~MessageListWidget()
	{

	}

	void MessageListWidget::setClientObject(ClientObjectPtr const& co)
	{
		Q_ASSERT(!_co);
		_co = co;
	}

	void MessageListWidget::insertItemWidget(ItemWidgetInterface* iwif, int row /*= -1*/)
	{
		QDateTime msgTime;
		if(count() != 0)
		{
			QListWidgetItem* item = nullptr;
			if(row == -1)
			{
				item = this->item(this->count() - 1);
			}
			else
			{
				item = this->item(0);
			}
			auto w = itemWidget(item);
			if(auto iwifLast = dynamic_cast<ItemWidgetInterface*>(w))
			{
				auto time = iwifLast->getDateTime();
				auto current = iwif->getDateTime();
				if(qAbs(time.secsTo(current)) > 300)
				{
					if(row == -1)
					{
						msgTime = current;
					}
					else
					{
						msgTime = time;
					}
				}
			}
		}
		else
		{

		}

		if(!msgTime.isNull())
		{
			insertCenterWidget(getTimeDisplayStringEx(msgTime), row);
		}

		auto item = new QListWidgetItem();
		if(row == -1)
		{
			addItem(item);
		}
		else
		{
			insertItem(row, item);
		}

		auto index = indexFromItem(item);
		iwif->attachItem(item, index);
		if(iwif->flags() & ItemWidgetInterface::outgoing)
		{
			if(_rightAvatar)
			{
				iwif->setAvatarPixmap(*_rightAvatar);
			}
		}

		updateItemWidgetSize(item, viewport()->size());
		if(row == -1)
		{
			scrollToBottom();
		}

		scheduleDelayedItemsLayout();
		//doItemsLayout();
	}

	ItemWidgetInterface* MessageListWidget::insertMessage(Jmcpp::MessagePtr const& msg, int row /*= -1*/)
	{
		auto msgId = msg->msgId;
		auto n = qMax(0, count() - 30);
		for(int i = count() - 1; i >= n; i--)
		{
			auto item = this->item(i);
			auto w = itemWidget(item);
			if(auto iwifLast = dynamic_cast<ItemWidgetInterface*>(w); iwifLast && iwifLast->getMessage())
			{
				if(iwifLast->getMessage()->msgId == msgId)
				{
					return nullptr;
				}
			}
		}

		ItemWidgetInterface* iw = nullptr;
		std::visit([=, &iw](auto&& cont)
		{
			insertMessage(msg, cont, iw, row);
		}, msg->content);
		return iw;
	}


	void MessageListWidget::setLeftAvatar(std::optional<QPixmap> const& img)
	{
		_leftAvatar = img;
		if(_leftAvatar)
		{
			_leftAvatar = _leftAvatar->scaled(45, 45);
			for(auto i = 0, n = count(); i < n; i++)
			{
				auto item = this->item(i);
				auto w = this->itemWidget(item);
				auto iw = dynamic_cast<ItemWidgetInterface*>(w);
				if(iw && iw->flags() & ItemWidgetInterface::incoming)
				{
					iw->setAvatarPixmap(*_leftAvatar);
				}
				else
				{

				}
			}
		}
	}
	void MessageListWidget::setRightAvatar(std::optional<QPixmap> const& pixmap)
	{
		_rightAvatar = pixmap;
		if(_rightAvatar)
		{
			_rightAvatar = _rightAvatar->scaled(45, 45);
			for(auto i = 0, n = count(); i < n; i++)
			{
				auto item = this->item(i);
				auto w = this->itemWidget(item);
				auto iw = dynamic_cast<ItemWidgetInterface*>(w);
				if(iw && iw->flags() & ItemWidgetInterface::outgoing)
				{
					iw->setAvatarPixmap(*pixmap);
				}
				else
				{

				}
			}
		}
	}

	void MessageListWidget::setLeftDisplayName(std::optional<QString> const& name)
	{
		_leftDisplayName = name;
		if(_leftDisplayName)
		{
			for(auto i = 0, n = count(); i < n; i++)
			{
				auto item = this->item(i);
				auto w = this->itemWidget(item);
				auto iw = dynamic_cast<ItemWidgetInterface*>(w);
				if(iw && iw->flags() & ItemWidgetInterface::incoming)
				{
					iw->setDisplayName(*_leftDisplayName);
				}
				else
				{

				}
			}
		}
	}

	void MessageListWidget::updateMessageReceipts(Jmcpp::ReceiptsUpdatedEvent const& ev)
	{
		auto count = this->count();
		while(count > 0)
		{
			auto item = this->item(count - 1);
			auto w = itemWidget(item);
			if(auto iwifLast = dynamic_cast<ItemWidgetInterface*>(w))
			{
				auto msg = iwifLast->getMessage();
				if(msg)
				{
					if(auto iter = ev.receipts.find(msg->msgId); iter != ev.receipts.end())
					{
						msg->unreadUserCount = iter->second.unreadUserCount;
						iwifLast->setUnreadUserCount(iter->second.unreadUserCount);
					}
				}
			}
			count--;
		}
	}

	None MessageListWidget::sendMessageReceipts(QVector<ItemWidgetInterface*> const& visible, ItemWidgetInterface::ItemFlags flags)
	{
		auto self = this | qTrack;
		Jmcpp::ConversationId conId;
		std::vector<int64_t> msgIds;

		flags &= ItemWidgetInterface::type_mask;

		std::vector<Jmcpp::MessagePtr> msgs;
		for(auto&& iwif : visible)
		{
			if(auto msg = iwif->getMessage())
			{
				if(!conId.isValid())
				{
					conId = msg->conId;
				}

				if(msg->needReceipt && !msg->isOutgoing && (iwif->flags()&ItemWidgetInterface::type_mask & flags))
				{
					msgs.emplace_back(msg);
					msgIds.emplace_back(msg->msgId);
				}
			}
		}
		if(conId.isValid() && !msgIds.empty())
		{
			co_await _co->sendMessageReceipts(conId, msgIds);
			co_await self;
			for(auto&& msg : msgs)
			{
				msg->needReceipt = false;
			}
		}
	}

	std::unique_ptr<QMenu> MessageListWidget::createMenu(ItemWidgetInterface* iw)
	{
		auto menu = std::make_unique<QMenu>();
		auto item = iw->item();
		auto itemWidget = iw->asWidget() | qTrack;
		auto msg = iw->getMessage();

		if(msg && !msg->conId.isRoom() && iw->flags() & ItemWidgetInterface::outgoing)
		{
			menu->addAction(u8"撤回", this, [=]
			{
				if(itemWidget)
				{
					retractMessage(iw);
				}
			});
		}
		if(iw->flags() & ItemWidgetInterface::text)
		{
			menu->addAction(u8"复制", this, [=]
			{
				if(msg && std::holds_alternative<Jmcpp::TextContent>(msg->content))
				{
					auto clipboard = qApp->clipboard();
					auto text = std::get<Jmcpp::TextContent>(msg->content).text;
					clipboard->setText(text.data());
				}
			});
		}

		if(msg)
		{
			if(iw->flags() & (ItemWidgetInterface::text
							  | ItemWidgetInterface::image
							  | ItemWidgetInterface::file
							  | ItemWidgetInterface::usercard
							  | ItemWidgetInterface::location
							  ))
			{
				menu->addAction(u8"转发", this, [=]
				{
					auto conIds = SelectUserWidget::getConversationIds(_co, this->topLevelWidget());
					auto content = msg->content;
					auto mainWidget = MainWidget::getCurrentMainWidget();
					if(mainWidget)
					{
						for(auto&& conId : conIds)
						{
							auto newMsg = _co->buildMessage(conId, content);
							mainWidget->getOrCreateChatWidget(conId)->forwardMessage(newMsg);
						}
						if(!conIds.empty())
						{
							QMessageBox::information(this->topLevelWidget(), "", u8"转发成功", QMessageBox::Ok);
						}
					}
				});
			}

			menu->addAction(u8"删除", this, [=]
			{
				if(msg){
					if(itemWidget)
					{
						delete takeItem(row(iw->item()));
					}
					_co->deleteMessage(msg->conId, msg->msgId);
				}
			});
		}

		return menu;
	}

	void MessageListWidget::showUnreadUsers(int64_t msgId)
	{
		UnreadUsers::showUnreadUsers(_co, msgId, this->topLevelWidget());
	}


	Jmcpp::MessagePtr MessageListWidget::getFirstLoadedMessage() const
	{
		for(int i = 0, n = count(); i < n; i++)
		{
			auto item = this->item(i);
			auto w = itemWidget(item);
			if(auto iwif = dynamic_cast<ItemWidgetInterface*>(w); iwif && iwif->getMessage() && iwif->getMessage()->msgId)
			{
				return iwif->getMessage();
			}
		}

		return {};
	}

	Jmcpp::MessagePtr MessageListWidget::getLastLoadedMessage() const
	{
		for(int i = count() - 1; i >= 0; i--)
		{
			auto item = this->item(i);
			auto w = itemWidget(item);
			if(auto iwif = dynamic_cast<ItemWidgetInterface*>(w); iwif && iwif->getMessage() && iwif->getMessage()->msgId)
			{
				return iwif->getMessage();
			}
		}

		return {};
	}

	void MessageListWidget::updateItemWidgetSize(QListWidgetItem* item, QSize const& newSize)
	{
		auto iw = itemWidget(item);
		ItemWidgetInterface* iwif = dynamic_cast<ItemWidgetInterface*>(iw);
		if(iwif)
		{
			auto sz = iwif->itemWidgetSizeHint(newSize);
			item->setSizeHint(sz);
		}
	}

	void MessageListWidget::updateItemWidgetSize(QListWidgetItem* item)
	{
		auto iw = itemWidget(item);
		ItemWidgetInterface* iwif = dynamic_cast<ItemWidgetInterface*>(iw);
		auto newSize = viewport()->size();
		if(iwif)
		{
			auto sz = iwif->itemWidgetSizeHint(newSize);
			item->setSizeHint(sz);
		}
	}

	void MessageListWidget::setLoadingMessageIndicator(bool enable)
	{
		_loadIndicator->setVisible(enable);
		if(enable)
		{
			_loadIndicator->setMovie(ItemWidgetInterface::getProgressMovie());
			auto x = (this->viewport()->width() - _loadIndicator->width()) / 2;
			_loadIndicator->move(x, 10);
		}
		else
		{
			_loadIndicator->clear();
		}
	}

	void MessageListWidget::insertCenterWidget(QString const& str, int row)
	{
		auto iwif = new ItemWidgetCenter();
		iwif->setDateTime(QDateTime::currentDateTime());
		iwif->setString(str);

		auto item = new QListWidgetItem();

		if(row == -1)
		{
			addItem(item);
		}
		else
		{
			insertItem(row, item);
		}

		iwif->attachItem(item, indexFromItem(item));

		if(row == -1)
		{
			scrollToBottom();
		}
	}


	//////////////////////////////////////////////////////////////////////////
	None MessageListWidget::insertUserCard(Jmcpp::MessagePtr const& msg, Jmcpp::UserId const& userId, ItemWidgetInterface*&iwif, int row)
	{
		auto co = _co;
		if(msg->isOutgoing)
		{
			auto iw = new ItemWidgetUserCardRight() | qTrack;
			iw->setMessage(msg);
			iw->setCardUserId(userId);
			insertItemWidget(iw, row);
			iwif = iw;
			if(msg->msgId)
			{
				iw->setUnreadUserCount(msg->unreadUserCount);
			}
			try
			{
				auto imgTask = co->getCacheAvatar(userId);
				auto infoTask = co->getCacheUserInfo(userId);

				auto img = co_await imgTask;
				auto info = co_await infoTask;

				co_await iw;
				iw->setCardUserInfo(info);
				iw->setCardAvatar(img);
				iw->setComplete();
				co_return;
			}
			catch(...)
			{

			}
		}
		else
		{
			auto iw = new ItemWidgetUserCardLeft() | qTrack;
			iw->setMessage(msg);
			iw->setCardUserId(userId);
			iw->setUnreadUserCount(msg->unreadUserCount);
			insertItemWidget(iw, row);
			iwif = iw;
			setLeftAvatarAndDisplayname(iw, msg);
			try
			{
				auto imgTask = co->getCacheAvatar(userId);
				auto infoTask = co->getCacheUserInfo(userId);

				auto img = co_await imgTask;
				auto info = co_await infoTask;

				co_await iw;
				iw->setCardUserInfo(info);
				iw->setCardAvatar(img);
				iw->setComplete();
				co_return;
			}
			catch(...)
			{

			}
			co_await iw;
			iw->setFailed();
			co_return;
		}

	}

	//////////////////////////////////////////////////////////////////////////
	None MessageListWidget::insertMessage(Jmcpp::MessagePtr const& msg,
										  Jmcpp::TextContent const& text,
										  ItemWidgetInterface*&iwif, int row)
	{
		auto doc = QJsonDocument::fromJson(QByteArray::fromStdString(text.extras));
		if(doc.isObject())
		{
			auto data = doc.object().toVariantMap();
			if(data.contains("businessCard"))
			{
				auto username = data.value("userName").toString().toStdString();
				auto appKey = data.value("appKey").toString().toStdString();

				if(!username.empty())
				{
					insertUserCard(msg, { username,appKey }, iwif, row);
					co_return;
				}
			}
		}

		auto self = this | qTrack.resumeOnShow;
		auto co = _co;
		auto emoji = Emoji::getSingleton();
		auto html = emoji->toImage(text.text.data());

		if(msg->isOutgoing)
		{
			auto iw = new ItemWidgetTextRight();
			iw->setHtml(html);
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;
			if(msg->msgId)
			{
				iw->setUnreadUserCount(msg->unreadUserCount);
			}
		}
		else
		{
			auto iw = new ItemWidgetTextLeft() | qTrack;
			iw->setHtml(html);
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;
			setLeftAvatarAndDisplayname(iw, msg);
		}

		co_return;
	}

	None MessageListWidget::insertMessage(Jmcpp::MessagePtr const& msg, Jmcpp::ImageContent const& content, ItemWidgetInterface*&iwif, int row)
	{
		auto self = this | qTrack;
		auto co = _co;

		bool largeEmoji = false;
		auto extra = content.extras;
		auto json = QJsonDocument::fromJson(QByteArray::fromRawData(extra.data(), extra.size())).object();
		if(json.value("kLargeEmoticon") == "kLargeEmoticon" || !json.value("jiguang").isUndefined())
		{
			largeEmoji = true;
		}

		if(msg->isOutgoing)
		{
			auto iw = new ItemWidgetImageRight() | qTrack.resumeOnShow;
			iw->setImageHolder({ content.width,content.height });
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;
			if(msg->msgId)
			{
				iw->setUnreadUserCount(msg->unreadUserCount);
			}
			try
			{
				if(QString::fromStdString(content.format).toLower() == "gif")
				{
					auto filePath = co_await co->getImage(content);
					co_await iw;
					iw->setFilePath(filePath);
					auto mv = new QMovie(filePath);
					iw->setMovie(mv, { content.width,content.height });
					iw->updateItemWidgetSize();
					co_return;
				}
				else
				{
					auto filepath = co_await co->getImage(content);
					QImage img(filepath);
					if(largeEmoji)
					{
						img = img.scaledToWidth(120);
					}
					co_await iw;
					iw->setFilePath(filepath);
					iw->setImage(QPixmap::fromImage(img));
					iw->updateItemWidgetSize();
					co_return;
				}
			}
			catch(...)
			{

			}
			co_await iw;
			iw->setFailed();
			iw->updateItemWidgetSize();
		}
		else
		{
			auto iw = new ItemWidgetImageLeft() | qTrack.resumeOnShow;

			iw->setImageHolder({ content.width,content.height });
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;

			try
			{
				setLeftAvatarAndDisplayname(iw, msg);
				if(QString::fromStdString(content.format).toLower() == "gif")
				{
					auto filePath = co_await co->getImage(content);
					co_await iw;
					auto mv = new QMovie(filePath);
					iw->setFilePath(filePath);
					iw->setMovie(mv, { content.width,content.height });
					iw->updateItemWidgetSize();
					co_return;
				}
				else
				{
					auto filepath = co_await co->getImage(content);
					QImage img(filepath);
					if(largeEmoji)
					{
						img = img.scaledToWidth(120);
					}
					co_await iw;
					iw->setFilePath(filepath);
					iw->setImage(QPixmap::fromImage(img));
					iw->updateItemWidgetSize();
					co_return;
				}
				co_return;
			}
			catch(...)
			{

			}

			co_await iw;
			iw->setFailed();
			iw->updateItemWidgetSize();
		}
	}

	None MessageListWidget::insertMessage(Jmcpp::MessagePtr const& msg_, Jmcpp::FileContent const& content, ItemWidgetInterface*&iwif, int row)
	{
		auto self = this | qTrack;
		auto co = _co;
		auto msg = msg_;
		if(msg->isOutgoing)
		{
			auto iw = new ItemWidgetFileRight() | qTrack.resumeOnShow;
			iw->setFileName(content.fileName.data());
			iw->setFileSize(content.fileSize);
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;
			if(msg->msgId)
			{
				iw->setUnreadUserCount(msg->unreadUserCount);
			}

			try
			{
				auto filePath = co_await co->getFile(content);
				co_await iw;
				iw->setFilePath(filePath);

				if(msg->msgId)
				{
					iw->setComplete();
				}
				co_return;
			}
			catch(...)
			{
			}
		}
		else
		{
			auto iw = new ItemWidgetFileLeft() | qTrack.resumeOnShow;
			iw->setFileName(content.fileName.data());
			iw->setFileSize(content.fileSize);
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;

			try
			{
				setLeftAvatarAndDisplayname(iw, msg);
				auto filePath = co->filePath(content);
				if(QFile::exists(filePath))
				{
					iw->setFilePath(filePath);
					iw->setComplete();
				}
				else
				{
					connect(iw, &ItemWidgetFileLeft::downloadClicked, this, [=]()->None
					{
						try
						{
							co_await iw;
							iw->setProgress(0);
							auto filePath = co_await co->getFile(content, [iw](int progress)
							{
								if(iw)
								{
									iw->setProgress(progress);
								}
							});
							co_await iw;
							iw->setFilePath(filePath);
							iw->setComplete();
							co_return;
						}
						catch(...)
						{
						}
						co_await iw;
						iw->setFailed();
						QFile::remove(filePath);
					});
				}
				co_return;
			}
			catch(...)
			{
			}
			co_await iw;
			iw->setFailed();
		}
	}

	None MessageListWidget::insertMessage(Jmcpp::MessagePtr const& msg, Jmcpp::LocationContent const& content, ItemWidgetInterface*&iwif, int row)
	{
		auto self = this | qTrack;
		auto co = _co;
		if(msg->isOutgoing)
		{
			auto iw = new ItemWidgetLocationRight() | qTrack.resumeOnShow;
			iw->setLocation(content);
			iw->setImageHolder({ 300,200 });
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;
			if(msg->msgId)
			{
				iw->setUnreadUserCount(msg->unreadUserCount);
			}

			try
			{
				auto pixmap = co_await co->getStaticMap(content);
				co_await iw;
				iw->setImage(pixmap);
				co_return;
			}
			catch(...)
			{
			}
		}
		else
		{
			auto iw = new ItemWidgetLocationLeft() | qTrack.resumeOnShow;
			iw->setLocation(content);
			iw->setImageHolder({ 300,200 });
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;
			try
			{

				setLeftAvatarAndDisplayname(iw, msg);
				auto pixmap = co_await co->getStaticMap(content);
				co_await iw;
				iw->setImage(pixmap);
				co_return;
			}
			catch(...)
			{
			}
		}
	}


	None MessageListWidget::insertMessage(Jmcpp::MessagePtr const& msg, Jmcpp::VoiceContent const&content, ItemWidgetInterface*&iwif, int row)
	{
		auto self = this | qTrack;
		auto co = _co;
		if(msg->isOutgoing)
		{
			auto iw = new ItemWidgetVoiceRight() | qTrack.resumeOnShow;
			iw->setData(content.duration);
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;

			if(msg->msgId)
			{
				iw->setUnreadUserCount(msg->unreadUserCount);
			}

			try
			{
				auto url = co_await co->getResource(content.mediaId);
				co_await iw;
				iw->setVoiceUrl(QString::fromStdString(url));
				iw->setComplete();
				co_return;
			}
			catch(...)
			{
			}
		}
		else
		{
			auto iw = new ItemWidgetVoiceLeft() | qTrack.resumeOnShow;
			iw->setData(content.duration);
			iw->setMessage(msg);
			insertItemWidget(iw, row);
			iwif = iw;

			try
			{
				setLeftAvatarAndDisplayname(iw, msg);
				auto url = co_await co->getResource(content.mediaId);
				co_await iw;
				iw->setVoiceUrl(QString::fromStdString(url));
				co_return;
			}
			catch(...)
			{
			}

			co_await iw;
			iw->setFailed();
		}
	}

	None MessageListWidget::retractMessage(ItemWidgetInterface* iw)
	{
		auto item = iw->item();
		auto itemWidget = iw->asWidget() | qTrack;
		auto msg = iw->getMessage();
		if(msg && msg->msgId)
		{
			if(QDateTime::fromMSecsSinceEpoch(msg->time).secsTo(QDateTime::currentDateTime()) < 180)
			{
				auto self = this | qTrack;
				co_await _co->retractMessage(msg->msgId);
			}
			else
			{
				QMessageBox::warning(this->topLevelWidget(), u8"提示", u8"发送时间过长，不能撤回", QMessageBox::StandardButton::Ok);
			}
		}
	}

	None MessageListWidget::onRetractMessage(Jmcpp::MessageRetractedEvent const& e)
	{
		auto msgId = e.msgIds.front();
		for(auto row = count() - 1; row >= 0; row--)
		{
			auto item = this->item(row);
			if(auto w = this->itemWidget(item))
			{
				if(auto iw = dynamic_cast<ItemWidgetInterface*>(w))
				{
					auto msg = iw->getMessage();
					if(msg && msg->msgId == msgId)
					{
						delete takeItem(row);

						_co->deleteMessage(msg->conId, msg->msgId);

						if(_co->getCurrentUser() == e.fromUser)
						{
							insertCenterWidget(u8"你撤回了一条消息", row);
						}
						else
						{
							if(e.conId.isGroup())
							{
								auto self = this | qTrack;
								auto name = co_await _co->getUserDisplayName(e.fromUser);
								co_await self;
								insertCenterWidget(QString(u8"%1撤回了一条消息").arg(name), row);
							}
							else
							{
								insertCenterWidget(u8"对方撤回了一条消息", row);
							}
						}
						break;
					}
				}
			}
		}
	}



	void MessageListWidget::showEvent(QShowEvent *event)
	{
		_timer->start(300);
	}

	void MessageListWidget::hideEvent(QHideEvent *event)
	{
		for(auto&& index : _visibleIndex)
		{
			if(index.isValid())
			{
				auto item = itemFromIndex(index);
				auto widget = this->itemWidget(item);
				if(auto iwif = dynamic_cast<ItemWidgetInterface*>(widget))
				{
					iwif->visibleChanged(false);
				}
			}
		}

		_visibleIndex.clear();
	}

	void MessageListWidget::resizeEvent(QResizeEvent *event)
	{
		auto newSize = viewport()->size();
		for(auto i = 0, n = count(); i < n; i++)
		{
			auto item = this->item(i);
			updateItemWidgetSize(item, newSize);
		}
	}

	void MessageListWidget::updateVisibleItems()
	{
		QVector<QPersistentModelIndex> currentVisibleIndex;
		QModelIndex index = indexAt(QPoint(0, 0));
		if(index.isValid())
		{
			currentVisibleIndex.push_back(index);

			while(viewport()->rect().contains(QPoint(0, visualRect(index).y() + visualRect(index).height() + 1)))
			{
				index = indexAt(QPoint(0, visualRect(index).y() + visualRect(index).height() + 1));
				if(index.isValid())
				{
					currentVisibleIndex.push_back(index);
				}
				else
				{
					break;
				}
			}
		}

		//qDebug() << currentVisibleIndex.size() << "are visible";

		QVector<QPersistentModelIndex> diff;

		std::sort(_visibleIndex.begin(), _visibleIndex.end());
		std::sort(currentVisibleIndex.begin(), currentVisibleIndex.end());

		std::set_difference(_visibleIndex.begin(), _visibleIndex.end(),
							currentVisibleIndex.begin(), currentVisibleIndex.end(),
							std::back_inserter(diff));


		QVector<ItemWidgetInterface*> visible;
		QVector<ItemWidgetInterface*> invisible;
		for(auto&& index : diff)
		{
			if(index.isValid())
			{
				auto item = itemFromIndex(index);
				auto widget = this->itemWidget(item);
				if(auto iwif = dynamic_cast<ItemWidgetInterface*>(widget))
				{
					iwif->visibleChanged(false);
					invisible.push_back(iwif);
				}
			}
		}

		_visibleIndex = std::move(currentVisibleIndex);

		for(auto&& index : _visibleIndex)
		{
			if(index.isValid())
			{
				auto item = itemFromIndex(index);
				auto widget = this->itemWidget(item);
				if(auto iwif = dynamic_cast<ItemWidgetInterface*>(widget))
				{
					iwif->visibleChanged(true);

					if(iwif->flags() & ItemWidgetInterface::incoming)
					{
						visible.push_back(iwif);
					}
				}
			}
		}

		Q_EMIT visibleChanged(visible, invisible);
	}

}
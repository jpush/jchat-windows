#pragma once

#include <algorithm>
#include <QWidget>
#include <QMenu>
#include <QClipboard>
#include <QLabel>
#include "ui_MessageListWidget.h"

#include "ClientObject.h"

#include "ItemWidgetInterface.h"

namespace JChat
{
	class MessageListWidget : public QListWidget
	{
		Q_OBJECT

	public:
		MessageListWidget(QWidget *parent = Q_NULLPTR);

		~MessageListWidget();

		void setClientObject(ClientObjectPtr const& co);

		ClientObjectPtr const& getClientObject() const{ return _co; }


		void setLoadingMessageIndicator(bool enable);

		void insertCenterWidget(QString const&, int row = -1);

		void insertItemWidget(ItemWidgetInterface* iwif, int row = -1);


		JChat::ItemWidgetInterface* insertMessage(Jmcpp::MessagePtr const& msg, int row = -1);

		//////////////////////////////////////////////////////////////////////////
		void setLeftAvatar(std::optional<QPixmap> const& img);

		void setRightAvatar(std::optional<QPixmap> const& pixmap);

		void setLeftDisplayName(std::optional<QString> const& name);

		None onRetractMessage(Jmcpp::MessageRetractedEvent const& e);

		void updateMessageReceipts(Jmcpp::ReceiptsUpdatedEvent const& ev);



		None sendMessageReceipts(QVector<ItemWidgetInterface*> const& visible, ItemWidgetInterface::ItemFlags flags = ItemWidgetInterface::ItemFlag::type_mask);


		std::unique_ptr<QMenu> createMenu(ItemWidgetInterface* iw);

		void showUnreadUsers(int64_t msgId);


		int getUnreadMessageCount() const
		{
			return _unreadMsgCount;
		}

		void setUnreadMessageCount(int count)
		{
			_unreadMsgCount = count;
		}

		Jmcpp::MessagePtr getFirstLoadedMessage() const;


		Jmcpp::MessagePtr getLastLoadedMessage() const;

	protected:

		void updateItemWidgetSize(QListWidgetItem* item, QSize const& newSize);

		void updateItemWidgetSize(QListWidgetItem* item);


		void insertCenterWidget(QString const&, bool) = delete;



		template<class T>
		void insertMessage(Jmcpp::MessagePtr const& msg, const T &, ItemWidgetInterface*&iwif, int row)
		{
			//qDebug() << __FUNCSIG__;
		}

		None insertMessage(Jmcpp::MessagePtr const& msg, Jmcpp::TextContent const& text, ItemWidgetInterface*&iwif, int row);

		None insertMessage(Jmcpp::MessagePtr const& msg, Jmcpp::ImageContent const& content, ItemWidgetInterface*&iwif, int row);

		None insertMessage(Jmcpp::MessagePtr const& msg, Jmcpp::FileContent const& content, ItemWidgetInterface*&iwif, int row);

		None insertMessage(Jmcpp::MessagePtr const& msg, Jmcpp::LocationContent const&, ItemWidgetInterface*&iwif, int row);

		None insertMessage(Jmcpp::MessagePtr const& msg, Jmcpp::VoiceContent const&, ItemWidgetInterface*&iwif, int row);


		void insertMessage(Jmcpp::MessagePtr const& msg, Jmcpp::CustomContent const&, ItemWidgetInterface*&iwif, int row)
		{

		}

		None insertUserCard(Jmcpp::MessagePtr const& msg, Jmcpp::UserId const& userId, ItemWidgetInterface*&iwif, int row);

		None retractMessage(ItemWidgetInterface* iw);


		template<class T>
		None setLeftAvatarAndDisplayname(T iw, Jmcpp::MessagePtr const& msg)
		{
			auto co = _co;
			if(msg->conId.isUser())
			{
				if(_leftAvatar)
				{
					iw->setAvatarPixmap(*_leftAvatar);
				}
				if(_leftDisplayName)
				{
					iw->setDisplayName(*_leftDisplayName);
				}
			}
			else
			{
				try
				{
					auto pixmapTask = co->getCacheUserAvatar(msg->sender);
					auto nameTask = co->getUserDisplayName(msg->sender);

					auto name = co_await nameTask;
					auto pixmap = co_await pixmapTask;

					co_await iw;
					iw->setDisplayName(name);
					iw->setAvatarPixmap(pixmap);
				}
				catch(...)
				{
				}
			}
		}

		Q_SIGNAL void visibleChanged(QVector<ItemWidgetInterface*> const& visible, QVector<ItemWidgetInterface*> const& invisible);

	protected:
		virtual void resizeEvent(QResizeEvent *event) override;
		virtual void showEvent(QShowEvent *event) override;
		virtual void hideEvent(QHideEvent *event) override;

		void updateVisibleItems();

	private:
		Ui::MessageListWidget ui;

		ClientObjectPtr	_co;

		std::optional<QPixmap>	_leftAvatar, _rightAvatar;
		std::optional<QString>	_leftDisplayName;
		QDateTime				_lastTime;

		bool					_allMessageLoaded = false;
		QLabel*					_loadIndicator = nullptr;


		int	_unreadMsgCount = 0;

		QTimer* _timer;
		QVector<QPersistentModelIndex> _visibleIndex;
	};
}






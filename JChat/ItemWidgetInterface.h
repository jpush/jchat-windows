#pragma once

#include <QFlags>
#include <QDateTime>
#include <QSize>
#include <QWidget>
#include <QListWidget>
#include <QMovie>

#include <Jmcpp/Messages.h>

namespace JChat
{
	class ItemWidgetInterface
	{
	public:
		enum ItemFlag
		{
			text = 1 << 0,
			image = 1 << 1,
			file = 1 << 2,
			location = 1 << 3,
			voice = 1 << 4,

			usercard = 1 << 7,

			//
			notice = 1 << 8,
			time = notice,

			type_mask = 0xFFFF,

			//
			outgoing = 1 << 16,
			incoming = 1 << 17,
		};

		Q_DECLARE_FLAGS(ItemFlags, ItemFlag);

		virtual ItemFlags  flags() = 0;

		virtual void setDisplayName(QString const&);;

		virtual void setAvatarPixmap(QPixmap const&);;

		virtual void		setDateTime(QDateTime const& dt);;
		virtual QDateTime	getDateTime() const;;

		//
		virtual void setProgress(int);;

		virtual void setComplete();;


		virtual void setFailed();;

		//recalculate
		virtual	QSize	itemWidgetSizeHint(QSize const& newViewportSize) = 0;

		virtual void	visibleChanged(bool isVisible);

		virtual void	setUnreadUserCount(size_t count);


		QWidget* asWidget();

		void attachItem(QListWidgetItem* item, QModelIndex const& index);

		QListWidgetItem* item() const;

		Jmcpp::MessagePtr const&	getMessage() const;
		void						setMessage(Jmcpp::MessagePtr const& msg);

		void	updateItemWidgetSize();

		QString getUnreadCountText(size_t count) const;

		static QMovie* getProgressMovie();

		static QPixmap getFailedPixmap();
		static QPixmap getFailedImagePlaceHolder();

	protected:
		Jmcpp::MessagePtr		_msg;
		QListWidgetItem*		_item = nullptr;
		QDateTime				_dt;
		QPersistentModelIndex  _index;
	};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(JChat::ItemWidgetInterface::ItemFlags)


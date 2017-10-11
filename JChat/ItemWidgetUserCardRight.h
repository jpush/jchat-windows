#pragma once

#include <QWidget>
#include "ui_ItemWidgetUserCardRight.h"
#include "ItemWidgetInterface.h"

namespace JChat {

	class ItemWidgetUserCardRight : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetUserCardRight(QWidget *parent = Q_NULLPTR);
		~ItemWidgetUserCardRight();

		virtual ItemFlags flags() override;


		virtual QSize itemWidgetSizeHint(QSize const& newViewportSize) override;


		virtual void setAvatarPixmap(QPixmap const&) override;


		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int) override;

		//////////////////////////////////////////////////////////////////////////

		void setCardUserId(Jmcpp::UserId const& userId);

		void setCardUserInfo(Jmcpp::UserInfo const& info);

		void setCardAvatar(QPixmap const& pixmap);

		virtual void setUnreadUserCount(size_t count) override;


		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ItemWidgetUserCardRight ui;

		Jmcpp::UserId _userId;
	};

} // namespace JChat

#pragma once

#include <QWidget>
#include "ui_ItemWidgetUserCardLeft.h"
#include "ItemWidgetInterface.h"

namespace JChat {

	class ItemWidgetUserCardLeft : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetUserCardLeft(QWidget *parent = Q_NULLPTR);
		~ItemWidgetUserCardLeft();


		virtual ItemFlags flags() override;


		virtual QSize itemWidgetSizeHint(QSize const& newViewportSize) override;


		virtual void setAvatarPixmap(QPixmap const&) override;


		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int) override;


		virtual void setDisplayName(QString const&) override;

		//////////////////////////////////////////////////////////////////////////
		void setCardUserId(Jmcpp::UserId const& userId);

		void setCardUserInfo(Jmcpp::UserInfo const& info);

		void setCardAvatar(QPixmap const& pixmap);

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ItemWidgetUserCardLeft ui;
		Jmcpp::UserId _userId;
	};

} // namespace JChat

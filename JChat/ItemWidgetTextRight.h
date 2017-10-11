#pragma once

#include <QWidget>
#include "ui_ItemWidgetTextRight.h"

#include "ItemWidgetInterface.h"

namespace JChat
{
	class ItemWidgetTextRight : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetTextRight(QWidget *parent = Q_NULLPTR);
		~ItemWidgetTextRight();


		void setHtml(QString const& );

		//
		virtual QSize itemWidgetSizeHint(QSize const& newSize) override;

		virtual ItemFlags  flags() override
		{
			return ItemFlag::text | ItemFlag::outgoing;
		}


		virtual void setAvatarPixmap(QPixmap const&) override;


		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int) override;


		virtual void setUnreadUserCount(size_t count) override;

	protected:

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ItemWidgetTextRight ui;
	};

}
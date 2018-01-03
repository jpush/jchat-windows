#pragma once

#include <QWidget>
#include "ui_ItemWidgetTextLeft.h"

#include "ItemWidgetInterface.h"

namespace JChat
{
	class ItemWidgetTextLeft : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetTextLeft(QWidget *parent = Q_NULLPTR);
		~ItemWidgetTextLeft();


		void setHtml(QString const& );


		//
		virtual QSize itemWidgetSizeHint(QSize const& newSize) override;

		virtual ItemFlags  flags() override
		{
			return  ItemFlag::text | ItemFlag::incoming;
		}

		virtual void setDisplayName(QString const&) override;

		virtual void setAvatarPixmap(QPixmap const&) override;


		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int) override;

	protected:

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ItemWidgetTextLeft ui;
	};

}


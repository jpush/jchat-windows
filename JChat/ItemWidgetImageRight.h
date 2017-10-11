#pragma once

#include <QWidget>
#include "ui_ItemWidgetImageRight.h"
#include "ItemWidgetInterface.h"


namespace JChat
{
	class ItemWidgetImageRight : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetImageRight(QWidget *parent = Q_NULLPTR);
		~ItemWidgetImageRight();


		//
		void setImageHolder(QSize const& sz);


		void setFilePath(QString const& filePath);

		void setImage(QPixmap const& img);
		void setMovie(QMovie *mv,QSize const& sz);


		//
		virtual QSize	itemWidgetSizeHint(QSize const& newSize) override;

		virtual ItemFlags  flags() override
		{
			return ItemFlag::image | ItemFlag::outgoing;
		}

		virtual void setAvatarPixmap(QPixmap const&pixmap) override
		{
			ui.label->setPixmap(pixmap);
		}


		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int) override;


		virtual void setUnreadUserCount(size_t count) override;


		virtual void visibleChanged(bool isVisible) override;

	protected:
		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ItemWidgetImageRight ui;
	};

}
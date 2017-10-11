#pragma once

#include <QWidget>
#include "ui_ItemWidgetImageLeft.h"
#include "ItemWidgetInterface.h"

namespace JChat
{
	class ItemWidgetImageLeft : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetImageLeft(QWidget *parent = Q_NULLPTR);
		~ItemWidgetImageLeft();


		//
		void setImageHolder(QSize const& imageSize);
		void setFilePath(QString const& filePath);
		void setImage(QPixmap const& img);
		void setMovie(QMovie *mv, QSize const& sz);

		//
		virtual QSize itemWidgetSizeHint(QSize const& newSize) override;


		virtual ItemFlags  flags() override
		{
			return ItemFlag::image | ItemFlag::incoming;
		}

		virtual void setAvatarPixmap(QPixmap const&) override;


		virtual void setDisplayName(QString const&) override;


		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int) override;


		virtual void visibleChanged(bool isVisible) override;


		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ItemWidgetImageLeft ui;
	};

}
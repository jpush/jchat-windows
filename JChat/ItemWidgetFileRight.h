#pragma once

#include <QWidget>
#include <QFileInfo>
#include "ui_ItemWidgetFileRight.h"
#include "ItemWidgetInterface.h"


namespace JChat
{
	class ItemWidgetFileRight : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetFileRight(QWidget *parent = Q_NULLPTR);
		~ItemWidgetFileRight();



		void setFilePath(QString const& filePath)
		{
			ui.frameF->setProperty("filepath", filePath);
		}


		void setFileName(QString const&fileName);

		void setFileSize(int64_t fileSize);

		//

		virtual ItemFlags  flags() override
		{
			return ItemFlag::file | ItemFlag::outgoing;
		}

		virtual QSize itemWidgetSizeHint(QSize const& newSize) override;

		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int) override;


		//
		virtual void setAvatarPixmap(QPixmap const&) override;




		virtual void setUnreadUserCount(size_t count) override;


		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ItemWidgetFileRight ui;
	};

}
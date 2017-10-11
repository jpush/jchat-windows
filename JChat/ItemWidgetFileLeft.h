#pragma once

#include <QWidget>
#include "ui_ItemWidgetFileLeft.h"
#include "ItemWidgetInterface.h"

namespace JChat {

	class ItemWidgetFileLeft : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetFileLeft(QWidget *parent = Q_NULLPTR);
		~ItemWidgetFileLeft();


		Q_SIGNAL void downloadClicked();

		void setFilePath(QString const& filePath)
		{
			ui.frameF->setProperty("filepath", filePath);
		}


		void setFileName(QString const&fileName)
		{
			ui.labelFileName->setText(fileName);
		}

		void setFileSize(int64_t fileSize)
		{
			ui.labelFileSize->setText(QString("%1KB").arg(fileSize / 1000));
		}


		//////////////////////////////////////////////////////////////////////////
		virtual ItemFlags  flags()
		{
			return ItemFlag::file| ItemFlag::incoming;
		}


		virtual QSize itemWidgetSizeHint(QSize const& newSize) override;


		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int value) override;


		virtual void setAvatarPixmap(QPixmap const&) override;

		virtual void setDisplayName(QString const&) override;


		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ItemWidgetFileLeft ui;
	};

} // namespace JChat

#pragma once

#include <QWidget>
#include "ui_ItemWidgetVoiceLeft.h"
#include "ItemWidgetInterface.h"

#include "ClientObject.h"

namespace JChat {

	class ItemWidgetVoiceLeft : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetVoiceLeft(QWidget *parent = Q_NULLPTR);
		~ItemWidgetVoiceLeft();


		void setData(uint32_t duration)
		{
			ui.labelDuration->setText(QString("%1\"").arg(duration));
		}

		void setVoiceUrl(QString const& url);

		virtual ItemFlags  flags() override
		{
			return ItemFlag::voice | ItemFlag::incoming;
		}

		virtual QSize itemWidgetSizeHint(QSize const& newSize) override;


		virtual void setDisplayName(QString const&) override;

		virtual void setAvatarPixmap(QPixmap const&) override;


		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int) override;

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	protected:

		void setPlaying(bool);

	private:
		Ui::ItemWidgetVoiceLeft ui;
	};

} // namespace JChat

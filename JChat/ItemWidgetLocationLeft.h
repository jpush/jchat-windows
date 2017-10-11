#pragma once

#include <QWidget>
#include "ui_ItemWidgetLocationLeft.h"
#include "ItemWidgetInterface.h"

#include "ClientObject.h"

namespace JChat {

	class ItemWidgetLocationLeft : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetLocationLeft(QWidget *parent = Q_NULLPTR);
		~ItemWidgetLocationLeft();


		virtual ItemFlags  flags()
		{
			return ItemFlag::location | ItemFlag::incoming;
		}

		void setLocation(Jmcpp::LocationContent const& location)
		{
			_location = location;
		}

		//
		void setImageHolder(QSize const& imageSize);

		void setImage(QPixmap const& img);

		//
		virtual QSize itemWidgetSizeHint(QSize const& newSize) override;


		virtual void setAvatarPixmap(QPixmap const&) override;


		virtual void setDisplayName(QString const&) override;


		virtual void setComplete() override;


		virtual void setFailed() override;


		virtual void setProgress(int) override;

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ItemWidgetLocationLeft ui;

		Jmcpp::LocationContent _location;
	};

} // namespace JChat

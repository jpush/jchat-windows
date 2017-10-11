#pragma once

#include <QWidget>
#include "ui_ItemWidgetCenter.h"
#include "ItemWidgetInterface.h"

namespace JChat {

	class ItemWidgetCenter : public QWidget, public ItemWidgetInterface
	{
		Q_OBJECT

	public:
		ItemWidgetCenter(QWidget *parent = Q_NULLPTR);
		~ItemWidgetCenter();



		void setString(QString const&str) const
		{
			ui.label->setText(str);
		}


		//
		virtual ItemFlags flags() override;

		virtual QSize itemWidgetSizeHint(QSize const& newSize) override;

	private:
		Ui::ItemWidgetCenter ui;
	};

} // namespace JChat

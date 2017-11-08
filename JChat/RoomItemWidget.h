#pragma once

#include <QWidget>
#include "ui_RoomItemWidget.h"

namespace JChat {

	class RoomItemWidget : public QWidget
	{
		Q_OBJECT

			Q_PROPERTY(QPixmap avatar READ getAvatar WRITE setAvatar)
			Q_PROPERTY(QString title READ getTitle WRITE setTitle)
			Q_PROPERTY(QString desc READ getDesc WRITE setDesc)

	public:
		Q_INVOKABLE RoomItemWidget(QWidget *parent = Q_NULLPTR);
		~RoomItemWidget();


		void	setAvatar(QPixmap const& pixmap)	{ ui.labelAvatar->setPixmap(pixmap); }
		QPixmap getAvatar() const{ return *ui.labelAvatar->pixmap(); }


		QString getTitle() const{ return ui.labelTitle->text(); }
		void	setTitle(QString const& text) { ui.labelTitle->setText(text); }


		QString getDesc() const{ return ui.labelDesc->text(); }
		void	setDesc(QString const& text) { ui.labelDesc->setText(text); }
	private:
		Ui::RoomItemWidget ui;
	};

} // namespace JChat

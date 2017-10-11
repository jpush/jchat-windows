#pragma once

#include <QWidget>
#include "ui_ChatIdItemWidget.h"

namespace JChat {

	class ChatIdItemWidget : public QWidget
	{
		Q_OBJECT

			Q_PROPERTY(QPixmap avatar READ getAvatar WRITE setAvatar)
			Q_PROPERTY(QString title READ getTitle WRITE setTitle)


			Q_PROPERTY(bool canCheck READ canCheck WRITE setCanCheck)
			Q_PROPERTY(bool canClose READ canClose WRITE setCanClose)

	public:

		enum ItemRole
		{
			ConId = Qt::UserRole + 333,
			Avatar,
			Title,
			CanCheck,
			CanClose,
		};


		Q_INVOKABLE ChatIdItemWidget(QWidget *parent = Q_NULLPTR);
		~ChatIdItemWidget();


		Q_SLOT void on_checkBox_toggled(bool checked);
		Q_SLOT void on_btnClose_clicked();


		Q_SIGNAL void toggled(bool checked);

		Q_SIGNAL void closeClicked();


		bool	isChecked() const{ return ui.checkBox->isChecked(); }

		void	setChecked(bool checkd){ ui.checkBox->setChecked(checkd); }

		QCheckBox* checkBox() const{ return ui.checkBox; }

	protected:

		void	setAvatar(QPixmap const& pixmap)	{ ui.labelAvatar->setPixmap(pixmap); }
		QPixmap getAvatar() const{ return *ui.labelAvatar->pixmap(); }


		QString getTitle() const{ return ui.labelTitle->text(); }
		void	setTitle(QString const& text) { ui.labelTitle->setText(text); }

		bool	canCheck()const{ return ui.checkBox->isVisible(); }
		void	setCanCheck(bool enabled){
			ui.checkBox->setVisible(enabled);
		}

		bool	canClose()const{ return ui.btnClose->isVisible(); }

		void	setCanClose(bool enabled){
			ui.btnClose->setVisible(enabled);
		}


	private:
		Ui::ChatIdItemWidget ui;
	};

} // namespace JChat

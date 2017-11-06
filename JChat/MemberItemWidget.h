#pragma once

#include <QWidget>
#include "ui_MemberItemWidget.h"

namespace JChat {

	class MemberItemWidget : public QWidget
	{
		Q_OBJECT

			Q_PROPERTY(QPixmap avatar READ getAvatar WRITE setAvatar)
			Q_PROPERTY(QString title READ getTitle WRITE setTitle)

			Q_PROPERTY(bool isOwner READ isOwner WRITE setOwner)
			Q_PROPERTY(bool isSilent READ isSilent WRITE setSilent)
	public:
		Q_INVOKABLE MemberItemWidget(QWidget *parent = Q_NULLPTR);
		~MemberItemWidget();

		Q_SIGNAL void memberInfoClicked() const;

		bool isOwner() const{ return _isOwner; }
		void setOwner(bool b);

		bool isSilent() const{ return _isSilent; }
		void setSilent(bool b);


		void	setAvatar(QPixmap const& pixmap)	{ ui.labelAvatar->setPixmap(pixmap); }
		QPixmap getAvatar() const{ return *ui.labelAvatar->pixmap(); }


		QString getTitle() const{ return ui.labelTitle->text(); }
		void	setTitle(QString const& text) { ui.labelTitle->setText(text); }


		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::MemberItemWidget ui;

		bool _isOwner = false;
		bool _isSilent = false;
	};

} // namespace JChat

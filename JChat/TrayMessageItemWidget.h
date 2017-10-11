#pragma once

#include <QWidget>
#include "ui_TrayMessageItemWidget.h"

namespace JChat {

class TrayMessageItemWidget : public QWidget
{
	Q_OBJECT

		Q_PROPERTY(QPixmap avatar READ getAvatar WRITE setAvatar)
		Q_PROPERTY(QString title READ getTitle WRITE setTitle)
		Q_PROPERTY(QString message READ getMessage WRITE setMessage)

		Q_PROPERTY(int unreadMessageCount READ getUnreadMessageCount WRITE setUnreadMessageCount)

public:
	Q_INVOKABLE TrayMessageItemWidget(QWidget *parent = Q_NULLPTR);
	~TrayMessageItemWidget();



	void	setAvatar(QPixmap const& pixmap)	{ ui.labelAvatar->setPixmap(pixmap); }
	QPixmap getAvatar() const{ return *ui.labelAvatar->pixmap(); }


	QString getTitle() const{ return ui.labelTitle->text(); }
	void	setTitle(QString const& text) { ui.labelTitle->setText(text); }


	QString getMessage() const{ return ui.labelMsg->text(); }
	void	setMessage(QString const& text) { ui.labelMsg->setText(text); }

	void setUnreadMessageCount(int count);

	int getUnreadMessageCount() const { return _unreadCount; }
private:
	Ui::TrayMessageItemWidget ui;

	int _unreadCount = 0;

};

} // namespace JChat

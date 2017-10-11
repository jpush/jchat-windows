#pragma once

#include <QWidget>
#include "ui_ConversationItemWidget.h"

#include "Util.h"
namespace JChat {

	class ConversationItemWidget : public QWidget
	{
		Q_OBJECT

			Q_PROPERTY(QPixmap avatar READ getAvatar WRITE setAvatar)
			Q_PROPERTY(QString title READ getTitle WRITE setTitle)
			Q_PROPERTY(QString message READ getMessage WRITE setMessage)
			Q_PROPERTY(QDateTime dateTime READ getDateTime WRITE setDateTime)
			Q_PROPERTY(bool isShield READ isShield WRITE setShield)

			Q_PROPERTY(bool isNotDisturb READ isNotDisturb WRITE setNotDisturb)
			Q_PROPERTY(int unreadMessageCount READ getUnreadMessageCount WRITE setUnreadMessageCount)

			Q_PROPERTY(bool sticktop MEMBER _sticktop WRITE setSticktop NOTIFY sticktopChanged)

	public:
		Q_INVOKABLE ConversationItemWidget(QWidget *parent = Q_NULLPTR);
		~ConversationItemWidget();

		Q_SLOT void on_btnClose_clicked();

		Q_SIGNAL void closeClicked();

		Q_SIGNAL void sticktopChanged(bool enabled);


		void setSticktop(bool enabled);

	protected:

		void	setAvatar(QPixmap const& pixmap)	{ ui.labelAvatar->setPixmap(pixmap); }
		QPixmap getAvatar() const{ return *ui.labelAvatar->pixmap(); }


		QString getTitle() const{ return ui.labelTitle->text(); }
		void	setTitle(QString const& text) { ui.labelTitle->setText(text); }


		QString getMessage() const{ return ui.labelMsg->text(); }
		void	setMessage(QString const& text) { ui.labelMsg->setText(text); }

		QDateTime getDateTime() const{
			return ui.labelTime->property("datetime").toDateTime();
		}
		void	setDateTime(QDateTime const& time) {
			ui.labelTime->setText(getTimeDisplayString(time));
			ui.labelTime->setProperty("datetime", time);
		}

		bool isShield() const
		{
			return ui.label->isVisible();
		}

		void setShield(bool enabled)
		{
			ui.label->setVisible(enabled);
		}

		void setUnreadMessageCount(int count);

		int getUnreadMessageCount() const { return _unreadCount; }

		bool isNotDisturb() const{ return false; }
		void setNotDisturb(bool enabled);


	protected:

		virtual void enterEvent(QEvent *event) override;


		virtual void leaveEvent(QEvent *event) override;

	private:
		Ui::ConversationItemWidget ui;

		QLabel* _unreadCountLabel = nullptr;
		int _unreadCount = 0;
		bool _isNotDisturb = false;

		bool _sticktop = false;
	};

} // namespace JChat

#pragma once

#include <QWidget>
#include "ui_FriendEventItemWidget.h"


#include "ClientObject.h"

namespace JChat {

	class FriendEventItemWidget : public QWidget
	{
		Q_OBJECT

	public:
		FriendEventItemWidget(QWidget *parent = Q_NULLPTR);
		~FriendEventItemWidget();

		None setEvent(ClientObjectPtr co, FriendEventT et);

		Q_SIGNAL void infoClicked(Jmcpp::UserId const&,int64_t);

		Q_SIGNAL void passClicked(Jmcpp::UserId const&, int64_t);

		Q_SIGNAL void rejectClicked(Jmcpp::UserId const&, int64_t);


		void setStatusLabel(QString const&text)
		{
			ui.labelStatus->setText(text);
		}

		void setStatus(bool outgoing, FriendEventT::Status status)
		{
			ui.labelMessage->setVisible(!outgoing);

			if(!outgoing && status == FriendEventT::undone)
			{
				ui.labelStatus->hide();
				ui.toolButtonPass->show();
				ui.toolButtonReject->show();
			}
			else
			{
				ui.labelStatus->show();
				ui.toolButtonPass->hide();
				ui.toolButtonReject->hide();
			}
		}
	protected:
		void setPixmap(QPixmap const& pixmap)
		{
			ui.label->setPixmap(pixmap);
		}

		void setTitleLabel(QString const& text)
		{
			ui.labelTitle->setText(text);
		}

		void setMessageLabel(QString const&text)
		{
			ui.labelMessage->setText(text);
		}

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::FriendEventItemWidget ui;

		Jmcpp::UserId	_userId;
		int64_t			_id;

		friend class FriendEventListWidget;
	};

} // namespace JChat

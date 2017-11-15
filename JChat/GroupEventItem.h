#pragma once

#include <QWidget>
#include "ui_GroupEventItem.h"

#include "ClientObject.h"


namespace JChat {

	class GroupEventItem : public QWidget
	{
		Q_OBJECT

	public:
		GroupEventItem(QWidget *parent = Q_NULLPTR);
		~GroupEventItem();



		Q_SIGNAL void groupInfoClicked(Jmcpp::GroupId);

		Q_SIGNAL void infoClicked(Jmcpp::UserId const&);


		Q_SIGNAL void passClicked(Jmcpp::GroupId, int64_t eventId, Jmcpp::UserId const& user, Jmcpp::UserId const& fromUser);

		Q_SIGNAL void rejectClicked(Jmcpp::GroupId, int64_t eventId, Jmcpp::UserId const& user, Jmcpp::UserId const& fromUser);


		None setEvent(ClientObjectPtr co, GroupEventT et);

		
		void setStatus(GroupEventT et)
		{
			if(et.isReject)
			{
				ui.labelStatus->hide();
				ui.toolButtonPass->hide();
				ui.toolButtonReject->hide();
			}
			else
			{
				if(et.status == FriendEventT::undone)
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
		}

		void setStatusLabel(QString const&text)
		{
			ui.labelStatus->setText(text);
		}


		auto const& getGroupEventT() const
		{
			return _et;
		}

	private:
		Ui::GroupEventItem ui;

		GroupEventT _et;
	};

} // namespace JChat

#pragma once

#include <QListWidget>
#include "ui_GroupEventWidget.h"

#include "ClientObject.h"

#include "GroupEventItem.h"

namespace JChat {

class GroupEventWidget : public QListWidget
{
	Q_OBJECT

public:

	enum Role
	{
		Id = Qt::UserRole + 1
	};

	GroupEventWidget(QWidget *parent = Q_NULLPTR);
	~GroupEventWidget();

	void setClientObject(ClientObjectPtr const& co);
	ClientObjectPtr const& getClientObject() const{ return _co; }


	int				unreadCount() const;

	void			clearUnread();

	Q_SIGNAL void	unreadChanged(int count);

	Q_SLOT None onPassClicked(Jmcpp::GroupId, int64_t eventId, Jmcpp::UserId const& user, Jmcpp::UserId const& fromUser);
	Q_SLOT None onRejectClicked(Jmcpp::GroupId, int64_t eventId, Jmcpp::UserId const& user, Jmcpp::UserId const& fromUser);


	void updateItem(GroupEventT const& et);
protected:

	void loadFromDB();

	GroupEventItem* createItemWidget();

private:
	Ui::GroupEventWidget ui;

	ClientObjectPtr _co;

};

} // namespace JChat

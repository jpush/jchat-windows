#pragma once

#include <QWidget>
#include "ui_GroupEventWidget.h"

#include "ClientObject.h"

namespace JChat {

class GroupEventWidget : public QWidget
{
	Q_OBJECT

public:
	GroupEventWidget(QWidget *parent = Q_NULLPTR);
	~GroupEventWidget();

	void setClientObject(ClientObjectPtr const& co);
	ClientObjectPtr const& getClientObject() const{ return _co; }


	int				unreadCount() const;

	void			clearUnread();

	Q_SIGNAL void	unreadChanged(int count);


protected:

	void loadFromDB();

private:
	Ui::GroupEventWidget ui;

	ClientObjectPtr _co;

};

} // namespace JChat

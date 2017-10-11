#pragma once

#include <QWidget>
#include "ui_ItemWidgetLocationRight.h"
#include "ItemWidgetInterface.h"

#include "ClientObject.h"
namespace JChat {

class ItemWidgetLocationRight : public QWidget, public ItemWidgetInterface
{
	Q_OBJECT

public:
	ItemWidgetLocationRight(QWidget *parent = Q_NULLPTR);
	~ItemWidgetLocationRight();


	virtual ItemFlags  flags()
	{
		return ItemFlag::location | ItemFlag::outgoing;
	}

	void setLocation(Jmcpp::LocationContent const& location)
	{
		_location = location;
	}

	//
	void setImageHolder(QSize const& imageSize);

	void setImage(QPixmap const& img);

	void setUnreadUserCount(size_t count);
	//
	virtual QSize itemWidgetSizeHint(QSize const& newSize) override;


	virtual void setAvatarPixmap(QPixmap const&) override;

	virtual void setComplete() override;


	virtual void setFailed() override;


	virtual void setProgress(int) override;


	virtual bool eventFilter(QObject *watched, QEvent *event) override;
private:
	Ui::ItemWidgetLocationRight ui;

	Jmcpp::LocationContent _location;

};

} // namespace JChat

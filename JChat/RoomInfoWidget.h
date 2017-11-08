#pragma once

#include <QWidget>
#include "ui_RoomInfoWidget.h"

namespace JChat {

class RoomInfoWidget : public QWidget
{
	Q_OBJECT

public:
	RoomInfoWidget(QWidget *parent = Q_NULLPTR);
	~RoomInfoWidget();

private:
	Ui::RoomInfoWidget ui;
};

} // namespace JChat

#pragma once

#include <QWidget>
#include "ui_RoomInfoPage.h"

#include "ClientObject.h"
namespace JChat {

	class RoomInfoPage : public QWidget
	{
		Q_OBJECT

	public:
		RoomInfoPage(QWidget *parent = Q_NULLPTR);
		~RoomInfoPage();

		None setRoomId(ClientObjectPtr co, Jmcpp::RoomId roomId);


		Q_SLOT void on_btnJoinRoom_clicked() ;
	private:
		Ui::RoomInfoPage ui;

		ClientObjectPtr _co;
		Jmcpp::RoomId _roomId{};

		bool _fetching = false;
	};

} // namespace JChat

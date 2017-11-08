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

		None setRoomId(ClientObjectPtr co, Jmcpp::RoomId roomId)
		{
			auto self = this | qTrack;
			_roomId = roomId;

			if(_roomId == 0)
			{
				ui.labelRoomName->setText("");
				ui.labelRoomId->setText(QString(u8"聊天室ID:"));
				ui.labelMembers->setText(QString(u8"聊天室成员:"));
				ui.textBrowser->setText(QString(u8"介绍:"));
				co_return;
			}

			auto roomInfo = co_await co->getRoomInfo(roomId);

			co_await self;

			if(roomId == _roomId)
			{
				ui.labelRoomName->setText(roomInfo.roomName.data());

				ui.labelRoomId->setText(QString(u8"聊天室ID: %1").arg(roomId.get()));
				ui.labelMembers->setText(QString(u8"聊天室成员: %1人").arg(roomInfo.currentMemberCount));

				ui.textBrowser->setText(QString(u8"介绍: %1").arg(roomInfo.description.c_str()));
			}

		}
	private:
		Ui::RoomInfoPage ui;

		Jmcpp::RoomId _roomId{};

		bool _fetching = false;
	};

} // namespace JChat

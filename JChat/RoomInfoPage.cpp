#include "RoomInfoPage.h"

#include "MainWidget.h"

#include "BusyIndicator.h"

#include "scope.hpp"

namespace JChat {

	RoomInfoPage::RoomInfoPage(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		ui.labelMembers->hide();
	}

	RoomInfoPage::~RoomInfoPage()
	{
	}

	None RoomInfoPage::setRoomId(ClientObjectPtr co, Jmcpp::RoomId roomId)
	{
		auto self = this | qTrack;
		_roomId = roomId;
		_co = co;

		if(_roomId == 0)
		{
			ui.labelRoomName->setText("");
			ui.labelRoomId->setText(QString(u8"聊天室ID:"));
			ui.labelMembers->setText(QString(u8"聊天室成员:"));
			ui.textBrowser->setText(QString(u8"介绍:"));

			ui.btnJoinRoom->hide();
			co_return;
		}

		auto roomInfo = co_await co->getRoomInfo(roomId);

		co_await self;

		if(roomId == _roomId)
		{
			ui.btnJoinRoom->show();

			ui.labelRoomName->setText(roomInfo.roomName.data());

			ui.labelRoomId->setText(QString(u8"聊天室ID: %1").arg(roomId.get()));
			ui.labelMembers->setText(QString(u8"聊天室成员: %1人").arg(roomInfo.currentMemberCount));

			ui.textBrowser->setText(QString(u8"介绍: %1").arg(roomInfo.description.c_str()));
		}
	}

	Q_SLOT void RoomInfoPage::on_btnJoinRoom_clicked()
	{
		auto mw = MainWidget::getCurrentMainWidget();
		if(mw && _roomId.get() && _co)
		{
			try
			{
				BusyIndicator busy(this->topLevelWidget());
				auto roomId = _co->getCurrentRoomId();
				if(roomId.get() && _roomId != roomId)
				{
					qAwait(_co->leaveRoom(roomId));
					_co->setCurrentRoomId({});
					mw->removeChatWidget(roomId);
				}

				roomId = _co->getCurrentRoomId();

				SCOPE_EXIT(unset){ _co->setCurrentRoomId(roomId); };
				try
				{
					_co->setCurrentRoomId(_roomId);
					qAwait(_co->enterRoom(_roomId));
				}
				catch(Jmcpp::ServerException& e)
				{
					if(e.code() == 881507){} //member has in the chatroom
					else
					{
						throw;
					}
				}

				unset.release();

				mw->switchToConversation(_roomId);
			}
			catch(std::runtime_error& e)
			{

			}
		}
	}

} // namespace JChat

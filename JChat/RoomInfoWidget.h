#pragma once

#include <QWidget>
#include "ui_RoomInfoWidget.h"

#include "ClientObject.h"

namespace JChat {

	class RoomInfoWidget : public QWidget
	{
		Q_OBJECT

	public:
		RoomInfoWidget(ClientObjectPtr const& co, Jmcpp::RoomId roomId, QWidget *parent = Q_NULLPTR);
		~RoomInfoWidget();


		Q_SLOT void close();



	protected:

		Q_SLOT void on_btnLeaveRoom_clicked();

		None updateInfo();

		void showEvent(QShowEvent *event) override;

		void paintEvent(QPaintEvent *event);



	private:
		Ui::RoomInfoWidget ui;
		bool active = false;

		ClientObjectPtr _co;
		Jmcpp::RoomId	_roomId;
	};

} // namespace JChat

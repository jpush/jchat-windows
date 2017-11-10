#pragma once

#include <QTime>
#include <QWidget>
#include "ui_ChatWidget.h"

#include "ClientObject.h"

class QCompleter;

namespace JChat
{
	class EmojiPicker;
	class ChatWidget : public QWidget
	{
		Q_OBJECT

	public:
		ChatWidget(ClientObjectPtr const& co, Jmcpp::ConversationId const& conId, QWidget *parent = Q_NULLPTR);

		~ChatWidget();

		MessageListWidget* listWidget() const
		{
			return ui.listWidget;
		}

		QString getEditingText() const;

		Q_SLOT void on_btnName_clicked();

		Q_SLOT void on_btnSetting_clicked();
		Q_SLOT void on_btnAdd_clicked();

		Q_SLOT None on_btnSend_clicked();
		Q_SLOT void on_btnImage_clicked();
		Q_SLOT void on_btnFile_clicked();

		Q_SLOT void on_btnUserCard_clicked();


		Q_SLOT void on_textEdit_textChanged();


		None init();

		None forwardMessage(Jmcpp::MessagePtr const& msgPtr);

		None sendImage(QByteArray const& image);

		None sendImage(QImage const& image);

		None sendImage(QString const& filePath);

		None sendFile(QString const& filePath);

		None sendUserCard(Jmcpp::UserId userId);

		None sendLargeEmoji(QString const& filePath, QString const& emojiName);

		None loadMoreMessage(size_t count);

		None loadMessage(size_t count);

		void onInputtingStatusChanged(bool inputting);
	protected:
		void onEmojiSelected(QString const& emojiHtml);
		void onLargetEmojiSelected(QString const&);

		void setInputtingLable(bool inputting);


		None setInputtingStatus(bool inputting);



		None onSelfInfoUpdated(Jmcpp::UserId const& userId);

		None onUserInfoUpdated(Jmcpp::UserId const& userId);

		None onGroupInfoUpdated(Jmcpp::GroupId groupId);


		virtual void showEvent(QShowEvent *event) override;


		virtual void hideEvent(QHideEvent *event) override;

	protected:

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ChatWidget ui;

		QCompleter*			_completer = nullptr;
		QWidget*			_groupOrRoomInfo = nullptr;

		bool				_loadingMessage = false;

		QTime				_inputtingTime;
		QTimer	*			_inputtingTimer = nullptr;

		QTimer	*			_inputtingLableTimer = nullptr;


		ClientObjectPtr _co;
		Jmcpp::ConversationId _conId;

		EmojiPicker*	_emojiPicker;

	};
}

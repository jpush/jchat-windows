#pragma once

#include <QWidget>
#include "ui_ChatWidget.h"

#include "ClientObject.h"
#include "EmojiPicker.h"
#include "GroupInfoWidget.h"

class QCompleter;

namespace JChat
{
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




		None init();

		None forwardMessage(Jmcpp::MessagePtr const& msgPtr);

		None sendImage(QImage const& image);

		None sendImage(QString const& filePath);

		None sendFile(QString const& filePath);

		None sendUserCard(Jmcpp::UserId userId);

		None loadMoreMessage(size_t count);

		None loadMessage(size_t count);
	protected:
		void onEmojiSelected(QVariantMap const& info);

		None onSelfInfoUpdated(Jmcpp::UserId const& userId);

		None onUserInfoUpdated(Jmcpp::UserId const& userId);

		None onGroupInfoUpdated(int64_t groupId);


		virtual void showEvent(QShowEvent *event) override;


		virtual void hideEvent(QHideEvent *event) override;

	protected:

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		Ui::ChatWidget ui;

		GroupInfoWidget*	 _groupInfo = nullptr;
		QCompleter*			_completer = nullptr;

		bool				_loadingMessage = false;

		ClientObjectPtr _co;
		Jmcpp::ConversationId _conId;

		EmojiPicker*	_emojiPicker;

	};
}

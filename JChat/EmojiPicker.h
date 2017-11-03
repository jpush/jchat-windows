#pragma once

#include <QWidget>
#include "ui_EmojiPicker.h"


namespace JChat {

	class EmojiPicker : public QWidget
	{
		Q_OBJECT

	public:
		EmojiPicker(QWidget *parent = Q_NULLPTR);
		~EmojiPicker();

		Q_SIGNAL void emojiSelected(QString const& emojiHtml);

		Q_SIGNAL void largeEmojiSelected(QString const&);

	private:
		Ui::EmojiPicker ui;
	};

} // namespace JChat

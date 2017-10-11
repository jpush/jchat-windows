#pragma once

#include <QWidget>
#include "ui_FileManager.h"

namespace JChat {

	class FileManager : public QWidget
	{
		Q_OBJECT

	public:
		FileManager(QWidget *parent = Q_NULLPTR);
		~FileManager();


		Q_SLOT void close();

	protected:

		virtual bool eventFilter(QObject *watched, QEvent *event) override;

		virtual void showEvent(QShowEvent *event) override;
	private:
		Ui::FileManager ui;

		bool active = false;

	};

} // namespace JChat

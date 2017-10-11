#pragma once

#include <QWidget>
#include "ui_SendFile.h"

namespace JChat {

class SendFile : public QWidget
{
	Q_OBJECT

public:
	SendFile(QWidget *parent = Q_NULLPTR);
	~SendFile();


	static int exec(QString const& filePath, QWidget * parent);

protected:
	virtual void closeEvent(QCloseEvent *event) override;
	Q_SIGNAL void closed();

private:
	Ui::SendFile ui;
};

} // namespace JChat

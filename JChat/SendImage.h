#pragma once

#include <QWidget>
#include "ui_SendImage.h"

namespace JChat {

class SendImage : public QWidget
{
	Q_OBJECT

public:
	SendImage(QWidget *parent = Q_NULLPTR);
	~SendImage();


	static int exec(QImage const& image, QWidget * parent);


protected:
	Q_SIGNAL void closed();

	virtual void closeEvent(QCloseEvent *event) override;
private:
	Ui::SendImage ui;
};

} // namespace JChat

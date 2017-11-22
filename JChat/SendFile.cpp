#include "SendFile.h"

#include <QEventLoop>
#include <QFileInfo>

namespace JChat {

	SendFile::SendFile(QWidget *parent)
		: QWidget(parent, Qt::Window)
	{
		ui.setupUi(this);
		setWindowModality(Qt::ApplicationModal);
		setWindowFlags(Qt::Window | Qt::WindowType::CustomizeWindowHint | Qt::WindowType::WindowCloseButtonHint);

		this->setFixedSize(size());
	}

	SendFile::~SendFile()
	{
	}

	int SendFile::exec(QString const& filePath, QWidget * parent)
	{
		QEventLoop el;
		SendFile widget(parent);
		QFileInfo info(filePath);

		widget.ui.labelFileName->setText(info.fileName());
		widget.ui.labelFileSize->setText(QString(u8"ด๓ะก: %1KB").arg(info.size() / 1000));

		widget.show();

		connect(widget.ui.btnOK, &QPushButton::clicked, [&]
		{
			el.exit(0);
		});

		connect(widget.ui.btnCancel, &QPushButton::clicked, [&]
		{
			el.exit(1);
		});
		connect(&widget, &SendFile::closed, [&]
		{
			el.exit(1);
		});


		return el.exec();
	}

	void SendFile::closeEvent(QCloseEvent *event)
	{
		emit closed();
	}

} // namespace JChat

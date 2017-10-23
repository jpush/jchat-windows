#pragma once

#include <QWidget>

namespace JChat {

	class BusyIndicator : public QWidget
	{
		Q_OBJECT
	public:
		BusyIndicator(QWidget *parent = Q_NULLPTR);
		~BusyIndicator();

	protected:
		virtual bool eventFilter(QObject *watched, QEvent *event) override;

		virtual void showEvent(QShowEvent *event) override;

		virtual void closeEvent(QCloseEvent *event) override;

		virtual void paintEvent(QPaintEvent *event) override;
	private:
	};

} // namespace JChat

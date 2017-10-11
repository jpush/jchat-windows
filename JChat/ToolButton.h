#pragma once

#include <QToolButton>


class QLabel;

namespace JChat {

	class ToolButton : public QToolButton
	{
		Q_OBJECT

	public:
		ToolButton(QWidget *parent = Q_NULLPTR);
		~ToolButton();


		void setCount(int count);

		void addCount(int count);

	protected:


	private:

		int _count = 0;

		QLabel* _unread;
	};

} // namespace JChat
